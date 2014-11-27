#include "src/io/serial/SerialPortList.h"

#include <cassert>

#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QStringList>

#include "3rdparty/qserialdevice/src/qserialdeviceenumerator/serialdeviceenumerator.h"

#include "src/concurrent/threadUtil.h"


// ******************
// ** Construction **
// ******************

/**
 * Creates a new SerialPortList instance. This constructor is private (singleton
 * pattern) and may only be called on the GUI thread. An exception is thrown
 * otherwise.
 */
SerialPortList::SerialPortList ()
{
	_mutex=new QMutex (QMutex::Recursive);

	// SerialDeviceEnumerator may only be accessed on the GUI thread.
	assert (isGuiThread ());

	// Now that we know we're on the GUI thread, we can create the device
	// enumerator, initialize our local list of available ports and connect the
	// hasChanged signal.
	SerialDeviceEnumerator *deviceEnumerator=SerialDeviceEnumerator::instance ();
	connect (deviceEnumerator, SIGNAL (hasChanged            (const QStringList &)),
	         this            , SLOT   (availablePortsChanged (const QStringList &)));

	_availablePorts = QSet<QString>::fromList (deviceEnumerator->devicesAvailable ());
}

SerialPortList::~SerialPortList ()
{
	delete _mutex;
}


// ***************
// ** Singleton **
// ***************

/** The singleton instance pointer */
SerialPortList *SerialPortList::_instance;

/** A mutex to protect the singleton instance pointer */
QMutex *SerialPortList::_instanceMutex (new QMutex (QMutex::Recursive));


/**
 * Returns a pointer to the singleton instance, creating it if necessary.
 *
 * This method is thread safe.
 */
SerialPortList *SerialPortList::instance ()
{
	QMutexLocker instanceLocker (_instanceMutex);

	if (!_instance)
		createInstance ();

	return _instance;
}

/**
 * Creates the singleton instance if it does not exist. If it already exists,
 * this method does nothing.
 *
 * This method is thread safe; however, it must be called from the GUI thread
 * the first time and it's pretty much useless after that.
 */
void SerialPortList::createInstance ()
{
	// ...but that doesn't mean that we get out of making it thread safe: the
	// instance method calls this method all the time.
	QMutexLocker instanceLocker (_instanceMutex);

	if (!_instance)
		_instance=new SerialPortList;
}


// ******************
// ** Port inquiry **
// ******************

/**
 * Returns a set containing the available ports as QString.
 *
 * This method is thread safe.
 */
QSet<QString> SerialPortList::availablePorts ()
{
	QMutexLocker locker (_mutex);
	return _availablePorts;
}

/**
 * Returns true if the given port is available or false otherwise.
 *
 * This method is thread safe.
 */
bool SerialPortList::isPortAvailable (const QString &port, Qt::CaseSensitivity caseSensitivity)
{
	QMutexLocker locker (_mutex);

	// In order to be able to configure case sensitivity, we need to convert the
	// set to a QStringList.
	QStringList portList = _availablePorts.toList ();
	locker.unlock ();

	return portList.contains (port, caseSensitivity);
}

QString SerialPortList::getDescription (const QString &deviceName)
{
	QMutexLocker locker (_mutex);

	SerialDeviceEnumerator *deviceEnumerator=SerialDeviceEnumerator::instance ();
	deviceEnumerator->setDeviceName (deviceName);
	return deviceEnumerator->description ();
}


// ***************
// ** Port list **
// ***************

/**
 * Invoked whenever the list or ports changes.
 *
 * This method is thread safe.
 */
void SerialPortList::availablePortsChanged (const QStringList &ports)
{
	QMutexLocker locker (_mutex);

	QSet<QString> oldPorts = _availablePorts;
	QSet<QString> newPorts = QSet<QString>::fromList (ports);
	_availablePorts = newPorts;

	// Now unlock the mutex again before we emit any signals.
	locker.unlock ();

	// Emit a signal for the port set change
	emit portsChanged (newPorts);

	// Compute the set of added and removed ports
	QSet<QString> addedPorts   = newPorts - oldPorts;
	QSet<QString> removedPorts = oldPorts - newPorts;

	// Emit signals for each added and removed port
	foreach (const QString &port, addedPorts  ) { emit portAdded   (port); }
	foreach (const QString &port, removedPorts) { emit portRemoved (port); }
}
