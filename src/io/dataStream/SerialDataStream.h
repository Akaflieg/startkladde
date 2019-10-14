#ifndef SERIALDATASTREAM_H_
#define SERIALDATASTREAM_H_

#include <QStringList>
#include <QDateTime>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QTimer>

#include "src/io/dataStream/DataStream.h"

class QMutex;

class AbstractSerial;

/**
 * A DataStream implementation that receives data from a serial port.
 *
 * The port and baud rate are configured using the setPort method. After that,
 * the stream can be opened.
 *
 * This DataStream implementation may block on opening.
 *
 * This implementation uses the QSerialDevice library, for reasons detailed on
 * the SerialPort page of the startkladde Wiki.
 *
 * This class is thread safe.
 */
class SerialDataStream: public DataStream
{
	Q_OBJECT

	public:
		SerialDataStream (QObject *parent);
		virtual ~SerialDataStream ();

        void setPort (const QString &portName, int baudRate);

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
		QString _portName;
		int _baudRate;

		// Back-end
        QTimer* _timer;
        QSerialPort *_port;

	private slots:
        void timer_timeout ();
    	void port_dataReceived ();
        void port_errorOccurred (QSerialPort::SerialPortError);
};

#endif
