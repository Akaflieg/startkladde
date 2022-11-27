#include "TcpDataStream.h"

#include <QMutex>
#include <QMutexLocker>
#include <QTcpSocket>

/*
 * Note: this class is thread safe. Keep it that way.
 */

// ******************
// ** Construction **
// ******************

TcpDataStream::TcpDataStream (QObject *parent): DataStream (parent),
	_parameterMutex (new QMutex ()),
	_backEndMutex   (new QMutex ()),
	_port (0)
{
	// Create the socket and connect the required signals. _socket will be
	// deleted automatically by its parent (this).
	_socket=new QTcpSocket (this);
	connect (_socket, SIGNAL (readyRead           ()),
	         this   , SLOT   (socket_dataReceived ()));
    connect (_socket, SIGNAL (error        (QAbstractSocket::SocketError)),
             this   , SLOT   (socket_error (QAbstractSocket::SocketError)));
    connect (_socket, SIGNAL (stateChanged        (QAbstractSocket::SocketState)),
             this   , SLOT   (socket_stateChanged (QAbstractSocket::SocketState)));
}

TcpDataStream::~TcpDataStream ()
{
	delete _parameterMutex;
	delete _backEndMutex;
}


// ****************
// ** Parameters **
// ****************

/**
 * Sets the target host and port to use when the stream is opened the next time.
 *
 * This method is thread safe.
 */
void TcpDataStream::setTarget (const QString &host, uint16_t port)
{
	QMutexLocker parameterLocker (_parameterMutex);

	_host=host;
	_port=port;
}


// ************************
// ** DataStream methods **
// ************************

/**
 * Implementation of DataStream::openStream ().
 *
 * This method is thread safe.
 */
void TcpDataStream::openStream ()
{
	// Lock the parameter mutex and make a copy of the parameters. Unlock the
	// parameter mutex afterwards.
	QMutexLocker parameterLocker (_parameterMutex);
	QString host=_host;
	uint16_t port=_port;
	parameterLocker.unlock ();

	// Lock the back-end mutex and open the socket. This method does not call
	// the base class, so we do not explicitly have to unlock the mutex. It will
	// be unlocked when the method returns.
	QMutexLocker backEndLocker (_backEndMutex);

	// If the socket is currently open, close it.
	if (_socket->state () != QAbstractSocket::UnconnectedState)
		_socket->abort ();

	// Initiate the connection
	_socket->connectToHost (host, port, QIODevice::ReadOnly);
}

/**
 * Implementation of DataStream::closeStream ().
 *
 * This method is thread safe.
 */
void TcpDataStream::closeStream ()
{
	// Lock the back-end mutex and close the socket.
	QMutexLocker backEndLocker (_backEndMutex);
	_socket->abort ();
}

/**
 * Implementation of DataStream::streamParametersCurrent ().
 *
 * This method is thread safe.
 */
bool TcpDataStream::streamParametersCurrent ()
{
	QMutexLocker backEndLocker (_backEndMutex);
	if (!_socket->isOpen ())
		return false;
	QString activeHost = _socket->peerName ();
	int     activePort = _socket->peerPort ();
	backEndLocker.unlock ();

	QMutexLocker parameterLocker (_parameterMutex);
	QString configuredHost = _host;
	int     configuredPort = _port;
	parameterLocker.unlock ();

	return
		configuredHost == activeHost &&
		configuredPort == activePort;

	return false;
}

// *******************
// ** Socket events **
// *******************

/**
 * Called when data is received from the socket.
 *
 * This method is thread safe.
 */
void TcpDataStream::socket_dataReceived ()
{
	// Lock the back-end mutex and read the data rom the socket. Unlock the
	// mutex before calling the base class.

	// Lock the back-end mutex and read the data from the socket. Unlock the
	// mutex before calling the base class.
	QMutexLocker backEndLocker (_backEndMutex);
	QByteArray data=_socket->readAll ();

	backEndLocker.unlock ();
	dataReceived (data);
}

/**
 * Called when the socked state changes, e. g. when the connection is
 * established.
 *
 * This method is thread safe.
 */
void TcpDataStream::socket_stateChanged (QAbstractSocket::SocketState socketState)
{
	// This method does not access any properties and only calls thread safe
	// methods. It is therefore thread safe and does not need locking.

	//qDebug () << "TcpDataStream: socket state changed to" << socketState;

	if (socketState == QAbstractSocket::ConnectedState)
		streamOpened ();
}

/**
 * Called when the socket connection fails (while opening) or is lost (while
 * open).
 *
 * This method is thread safe.
 */
void TcpDataStream::socket_error (QAbstractSocket::SocketError error)
{
	// We ignore the error argument and instead use the socket's errorString
	// method to obtain an error message.
	Q_UNUSED (error);

	// Lock the back-end mutex, make sure that the socket is closed, and read
	// the error string. Unlock the mutex before calling the base class.
	QMutexLocker backEndLocker (_backEndMutex);

	//qDebug () << "TcpDataStream: socket error:" << error << "in socket state" << socket->state () ;
	_socket->abort ();
	QString errorString=_socket->errorString ();

	backEndLocker.unlock ();
	streamError (errorString);
}
