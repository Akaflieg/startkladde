#ifndef MANAGEDDATASTREAM_H_
#define MANAGEDDATASTREAM_H_

#include <QObject>
#include <QTime>

#include "src/io/dataStream/DataStream.h" // For DataStream::State

class QTimer;

class BackgroundDataStream;

/**
 * A wrapper around DataStream that provides automatic reconnecting on error and
 * data monitoring.
 *
 * Create a ManagedDataStream instance and set an underlying data stream by
 * calling the setDataStream method. You can then use the open and close methods
 * to open and close the stream, respectively. If the stream encounters an error
 * during or after connection, it will automatically reconnect after a delay.
 *
 * When no data is received for a given amount of time, the state is changed
 * accordingly.
 *
 * You can change or remove the underlying data stream at any time. If the
 * stream was open before, the new data stream will be opened automatically.
 *
 * When the state changes, the stateChanged signal is emitted. The meaning of
 * the individual states:
 *   - noConnection: no underlying data stream has been assigned (no connection
 *     is configured).
 *   - closed: the user has not requested the stream to be opened. All other
 *     states imply that the user has requested the stream to be opened.
 *   - opening: the connection is currently being established. This may be the
 *     initial connect after opening the stream, or a reconnect after a
 *     connection fails or is lost (see below).
 *     In this state, errorMessage may contain an error message string.
 *   - open: the connection has been established, but no data has been
 *     received yet. This state does not indicate an error, it is always reached
 *     after connecting successfully. If there is a data timeout after some data
 *     has been received, streamDataTimeout is used (see below).
 *   - ok: the connection has been established and data has recently been
 *     received.
 *   - dataTimeout: the connection is still active, but no data has been
 *     received for more than a given time interval.
 *   - error: the connection could either not been established or was
 *     lost after it had been established.
 *     In this state, errorMessage may contain an error message string.
 *     reconnectTime contains the time of the next connection attempt.
 *
 * Simplified state diagram (does not take into account setting/clearing the
 * underlying data stream):
 *
 *             (Start)
 *                |
 *            .--------.                       Close .---------------.
 *            | Closed |<----------------------------| Error         |
 *            '--------'                      ^      |---------------|
 *          Open | ^                          |      | errorMessage  |
 *               V | Close                    |      | reconnectTime |
 *         .--------------.        Reconnect timeout |               |
 *         | Connecting   |<-------------------------|               '
 *         |--------------| Close             |      '---------------'
 *         | errorMessage |------------------>|              ^
 *         |              |--------------------------------->|
 *         '--------------' Error             |              |
 *        Success |                           |              |
 *                V                           |              |
 *           .---------. Close                |              |
 *           | No data |--------------------->|              |
 *           |         |------------------------------------>|
 *           '---------' Error                |              |
 *  Data received |                           |              |
 *                V                           |              |
 *           .---------. Close                |              |
 *           | Data OK |--------------------->|              |
 *           |         |------------------------------------>|
 *           '---------' Error                |              |
 * Data Timeout |   ^                         |              |
 *              V   | Data received           |              |
 *         .--------------. Close             |              |
 *         | Data timeout |-------------------'              |
 *         |              |----------------------------------'
 *         '--------------' Error
 */
class ManagedDataStream: public QObject
{
	Q_OBJECT

	public:
		struct State
		{
			enum Type { noConnection, closed, opening, open, ok, timeout, error };
			static QString text (Type type);
		};

		struct ConnectionState
		{
			enum Type { closed, opening, open, error };
		};

		struct DataState
		{
			enum Type { none, ok, timeout };
		};

		// Construction
		ManagedDataStream (QObject *parent);
		virtual ~ManagedDataStream ();

		// State
		State::Type getState () const;
		QString getErrorMessage () const;
		QTime getReconnectTime () const;

		// Underlying data stream
		void setDataStream (DataStream *stream, bool streamOwned);
		void clearDataStream ();
		DataStream *getDataStream () const;

	public slots:
		// Opening/closing
		void open ();
		void close ();
		void setOpen (bool o);
		bool isOpen () const;

	signals:
		// Public interface
		/**
		 * Emitted whenever the state changes. Note that when receiving this
		 * signal, the state may already have changed again and the
		 * corresponding signal may have been delivered yet.
		 */
		void stateChanged (ManagedDataStream::State::Type state);
		/**
		 * Re-emitted from the underlying data stream. See
		 * DataStream::dataReceived.
		 */
		void dataReceived (QByteArray line);
		/**
		 * Re-emitted from the underlying data stream. See
		 * DataStream::connectionBecameAvailable.
		 */
		void connectionBecameAvailable ();

	protected:
		void goToConnectionState (ConnectionState::Type connectionState);
		void goToDataState       (DataState::Type       dataState      );
		void updateState ();
		State::Type determineState () const;
		void startReconnectTimer ();
		void stopReconnectTimer ();
		void setReconnectTimerRunning (bool running);

	private:
		BackgroundDataStream *_backgroundStream;

		// Internal state
		bool _open;
		ConnectionState::Type _connectionState;
		DataState::Type _dataState;
		// External state
		State::Type _state;

		// Internals
	    QTimer *_dataTimer;
	    QTimer *_reconnectTimer;
	    QTime _reconnectTime;

	private slots:
		void dataTimer_timeout ();
		void reconnectTimer_timeout ();

		void stream_stateChanged (DataStream::State state);
		void stream_dataReceived (QByteArray data);
		void stream_connectionBecameAvailable ();
};

#endif
