#ifndef TCPDATASTREAM_H_
#define TCPDATASTREAM_H_

#include <stdint.h>
#include <QAbstractSocket>

#include "src/io/dataStream/DataStream.h"

class QMutex;
class QTcpSocket;

/**
 * A DataStream implementation that receives data via a TCP socket.
 *
 * The host and port are configured using the setTarget method. After that, the
 * stream can be opened.
 *
 * This DataStream implementation will not block on opening.
 *
 * This class is thread safe.
 */
class TcpDataStream: public DataStream
{
	Q_OBJECT

	public:
		TcpDataStream (QObject *parent);
		virtual ~TcpDataStream ();

		void setTarget (const QString &host, uint16_t port);

	protected:
		// DataStream methods
		virtual void openStream ();
		virtual void closeStream ();
		virtual bool streamParametersCurrent ();

	private:
		// There are two different mutexes: one for protecting the parameters
		// and one for protecting the back-end. This is so that a blocking
		// back-end operation does not block other operations.
	    QMutex *_parameterMutex;
	    QMutex *_backEndMutex;

	    // Parameters
		QString _host;
		uint16_t _port;

		// Back-end
	    QTcpSocket *_socket;

	private slots:
    	void socket_dataReceived ();
		void socket_stateChanged (QAbstractSocket::SocketState socketState);
		void socket_error (QAbstractSocket::SocketError error);
};

#endif
