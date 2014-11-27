#include "src/util/DestructionMonitor.h"

#include <iostream>

#include <QString>
#include <QDebug>

#include "src/util/qString.h"

/**
 * Creates a destruction monitor for the given object and stores the message
 *
 * This can be useful during development to verify that an object is actually
 * deleted, for example, when deletion is performed automatically after the
 * object is no longer needed.
 *
 * Note that the object is not set as the parent of this destruction monitor: if
 * it was, the destruction monitor would be deleted by the object before
 * receiving the destroyed() signal.
 *
 * This class is not intended to be created directly by the user. Use the static
 * message() method to register an object.
 */
DestructionMonitor::DestructionMonitor (QObject *object, const QString &message):
	QObject (NULL),
	_object (object), _message (message)
{
	// Store the class name here. In the objectDestroyed slot, it always returns
	// QObject.
	_className=object->metaObject()->className ();
	_objectName=object->objectName ();
	connect (object, SIGNAL (destroyed ()), this, SLOT (objectDestroyed ()));
}

DestructionMonitor::~DestructionMonitor ()
{
}

/**
 * Invoked when the monitored object is destroyed. Prints a message to standard
 * error and schedules the deletion of this DestructionMoonitor instance.
 */
void DestructionMonitor::objectDestroyed ()
{
	QString text="Object destroyed: " + _className;

	// Add the object name, if it is set
	if (!_objectName.isEmpty ())
		text+=" "+_object->objectName ();

	// Add the message, if it is set
	if (!_message.isEmpty ())
		text+=" ("+_message+")";

	std::cerr << text << std::endl;

	this->deleteLater ();
}
