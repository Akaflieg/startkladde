/*
 * Improvements:
 *   - If the server port is closed, don't accept the client connection.
 *     However, the connection is accepted by QTcpServer
 *   - Make the error source (QAbstractSocket::SocketError socketError)
 *     available to the caller (and use it)
 *   - More sophisticated error handling: currently we just close the
 *     connection on any error (and let the auto-reconnect pick up)
 */
#include "TcpProxy.h"

#include <iostream>

#include "src/concurrent/synchronized.h"
#include "src/util/qString.h"
#include "src/concurrent/Returner.h"
#include "src/i18n/notr.h"

//#define DEBUG(stuff) do { std::cout << stuff << std::endl; } while (0)
#define DEBUG(stuff)

TcpProxy::TcpProxy ():
	server (NULL), serverSocket (NULL), clientSocket (NULL),
	readTimedOut (false), readTimeoutMs (0)
{
	DEBUG (notr ("Creating a TcpProxy on thread ") << QThread::currentThreadId ());

	connect (this, SIGNAL (sig_open (Returner<quint16> *, QString, quint16)), this, SLOT (slot_open (Returner<quint16> *, QString, quint16)));
	connect (this, SIGNAL (sig_close ()), this, SLOT (slot_close ()));

	moveToThread (&proxyThread);
	proxyThread.start ();
}

TcpProxy::~TcpProxy ()
{
	proxyThread.quit ();

	std::cout << notr ("Waiting for proxy thread to terminate...");
	std::cout.flush ();

	if (proxyThread.wait (1000))
		std::cout << notr ("OK") << std::endl;
	else
		std::cout << notr ("Timeout") << std::endl;

	// server, serverSocket and clientSocket will be deleted automatically
}

// ****************
// ** Properties **
// ****************

quint16 TcpProxy::getProxyPort ()
{
	synchronizedReturn (mutex, proxyPort);
}

/**
 * This may only be called before the proxy has been opened.
 * @param timeout
 */
void TcpProxy::setReadTimeout (int timeout)
{
	readTimeoutMs=timeout;
}

// ***************
// ** Frontends **
// ***************

quint16 TcpProxy::open (const QString &serverHost, quint16 serverPort)
{
	Returner<quint16> returner;
	emit sig_open (&returner, serverHost, serverPort);
	return returner.returnedValue ();
}

void TcpProxy::close ()
{
	emit sig_close ();
}

// **************
// ** Backends **
// **************

/**
 * Opens the proxy server, but not the connection to the (real) server
 */
void TcpProxy::slot_open (Returner<quint16> *returner, QString serverHost, quint16 serverPort)
{
	returnOrException (returner, openImpl (serverHost, serverPort));
}

quint16 TcpProxy::openImpl (QString serverHost, quint16 serverPort)
{
	// TODO only if host and port matches
	if (server && server->isListening ())
	{
		// Proxy already running

		if (this->serverHost==serverHost && this->serverPort==serverPort)
			// Correct server
			return server->serverPort ();
		else
			// Wrong server
			server->close ();
	}

	DEBUG (notr ("Open server in thread ") << QThread::currentThreadId ());

	// Store the connection data
	synchronized (mutex)
	{
		this->serverHost=serverHost;
		this->serverPort=serverPort;
	}

	delete server;
	server=new QTcpServer (this);

	connect (server, SIGNAL (newConnection ()), this, SLOT (newConnection ()));

	if (server->isListening ()) server->close ();

	if (server->listen (QHostAddress::LocalHost, 0))
	{
		synchronized (mutex) this->proxyPort=server->serverPort ();

		DEBUG (notr ("OK, listening on port ") << server->serverPort ());
	}
	else
	{
		DEBUG (notr ("Listen failed"));
	}

	return server->serverPort ();
}

/**
 * Closes the connection, if there is any
 */
void TcpProxy::slot_close ()
{
	readTimer.stop ();
	doClose ();
}

/**
 * Close the sockets, but don't stop the read timer.
 */
void TcpProxy::doClose ()
{
	DEBUG (notr ("Close connection in thread ") << QThread::currentThreadId ());
	closeClientSocket ();
	closeServerSocket ();
}

void TcpProxy::closeClientSocket ()
{
	DEBUG (notr ("close client socket"));
	if (!clientSocket) return;

	if (serverSocket) clientSocket->write (serverSocket->read (serverSocket->bytesAvailable ()));
	clientSocket->flush ();

	clientSocket->disconnect(); // signals
	clientSocket->close ();
	clientSocket->deleteLater ();
	clientSocket=NULL;
}

void TcpProxy::closeServerSocket ()
{
	DEBUG (notr ("close server socket"));

	if (!serverSocket) return;

	if (clientSocket) serverSocket->write (clientSocket->read (clientSocket->bytesAvailable ()));
	serverSocket->flush ();

	serverSocket->disconnect ();
	serverSocket->close ();
	serverSocket->deleteLater ();
	serverSocket=NULL;
}

void TcpProxy::newConnection ()
{
	DEBUG (notr ("new connection on thread ") << QThread::currentThreadId ());
	DEBUG (notr ("connecting to %1:%2").arg (serverHost).arg (serverPort));

	delete serverSocket;
	serverSocket=new QTcpSocket (this);
	serverSocket->connectToHost (serverHost, serverPort);

	delete clientSocket;
	clientSocket=server->nextPendingConnection ();

	connect (serverSocket, SIGNAL (readyRead ()), this, SLOT (serverRead ()));
	connect (clientSocket, SIGNAL (readyRead ()), this, SLOT (clientRead ()));

	connect (serverSocket, SIGNAL (disconnected ()), this, SLOT (serverClosed ()));
	connect (clientSocket, SIGNAL (disconnected ()), this, SLOT (clientClosed ()));

    connect (serverSocket, &QTcpSocket::errorOccurred, this, &TcpProxy::serverError);
    connect (clientSocket, &QTcpSocket::errorOccurred, this, &TcpProxy::clientError);

	resetTimer ();
}

void TcpProxy::clientRead ()
{
	DEBUG (notr ("write to server...") << clientSocket->bytesAvailable ());

	if (serverSocket) serverSocket->write (clientSocket->readAll ());

	DEBUG (notr ("done"));
}

void TcpProxy::serverRead ()
{
	DEBUG (notr ("write to client...") << serverSocket->bytesAvailable ());

	if (readTimedOut)
	{
		readTimedOut=false;
		emit readResumed ();
	}

	resetTimer ();

	if (clientSocket) clientSocket->write (serverSocket->readAll ());


	DEBUG (notr ("done"));
}

void TcpProxy::clientClosed ()
{
	DEBUG (notr ("client closed"));
	doClose ();
}

void TcpProxy::serverClosed ()
{
	DEBUG (notr ("server closed"));
	doClose ();
}

void TcpProxy::clientError ()
{
	DEBUG (notr ("client error"));
	doClose ();
}

void TcpProxy::serverError ()
{
	DEBUG (notr ("server error"));
	doClose ();
}


// **************
// ** Timeouts **
// **************

void TcpProxy::timerEvent (QTimerEvent *event)
{
	// Multiple timers: event->timerId == readTimer.timerId ()
	(void)event;

	if (!readTimedOut)
	{
		readTimedOut=true;
		emit readTimeout ();
	}
}

void TcpProxy::resetTimer ()
{
	readTimer.stop ();

	if (readTimeoutMs!=0)
		readTimer.start (readTimeoutMs, this);
}
