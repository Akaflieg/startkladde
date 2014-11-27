/*
 * TcpProxy.h
 *
 *  Created on: 06.03.2010
 *      Author: Martin Herrmann
 */

#ifndef TCPPROXY_H_
#define TCPPROXY_H_

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QThread>
#include <QMutex>
#include <QBasicTimer>

#include "src/concurrent/synchronized.h"

template<class T> class Returner;

class TcpProxy: public QObject
{
	Q_OBJECT

	public:
		TcpProxy ();
		virtual ~TcpProxy ();

		quint16 open (const QString &serverHost, quint16 serverPort);
		void close ();

		quint16 getProxyPort ();

		void setReadTimeout (int timeout);

	signals:
		void sig_open (Returner<quint16> *returner, QString serverHost, quint16 serverPort);
		void sig_close ();

		void readTimeout ();
		void readResumed ();

	protected slots:
		void slot_open (Returner<quint16> *returner, QString serverHost, quint16 serverPort);
		void slot_close ();

		quint16 openImpl (QString serverHost, quint16 serverPort);

		virtual void newConnection ();

		virtual void clientRead ();
		virtual void serverRead ();

		virtual void clientClosed ();
		virtual void serverClosed ();

		virtual void clientError ();
		virtual void serverError ();

	protected:
		virtual void closeServerSocket ();
		virtual void closeClientSocket ();

		void timerEvent (QTimerEvent *event);
		void resetTimer ();

		void doClose ();

	private:
		QMutex mutex;

		QTcpServer *server;
		QTcpSocket *serverSocket;
		QTcpSocket *clientSocket;

		QThread proxyThread;

		QString serverHost;
		quint16 serverPort;
		quint16 proxyPort;

		QBasicTimer readTimer;
		bool readTimedOut;
		int readTimeoutMs;
};

#endif
