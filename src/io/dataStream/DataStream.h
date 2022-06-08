#ifndef DATASTREAM_H_
#define DATASTREAM_H_

#include <QObject>

class QRecursiveMutex;
class QTimer;

/**
 * The abstract base class for DataStream implementations
 *
 * The DataStream architecture provides a common interface to different
 * communication channels, such as serial, network, or reading from a file.
 * Additionally, the interface consists mostly of signals and slots, which
 * facilitates running a DataStream instance on a background thread.
 *
 * At any given time, a data stream is in one of three states:
 *   - closed: the data stream has not been opened, or it has been closed, or
 *     it was closed due to failure (e. g. TCP connection terminated)
 *   - opening: the data stream has been requested to open, but is not open yet
 *     (e. g. a TCP data stream waiting for the remote side to accept the
 *     connection). An opening stream will go to the open state if the
 *     connection succeeds, or to the closed state if the connection fails
 *     (e. g. the remote side refusing a TCP connection).
 *   - open: the data stream has successfully been opened
 *
 * A data stream is configured (e. g. setting the remote host and port for a TCP
 * data stream) using implementation-specific methods. The parameters are
 * applied when the connection is opened the next time. If the stream is already
 * open, the parameters are not applied immediately.
 *
 * The stream is then opened by calling the `open` method, causing it to go to
 * the `opening` state (before the `open` method returns). After that, it will
 * either go to the `open` state (on success) or to the `closed` state (on
 * failure). Depending on the implementation, this may either happen immediately
 * (before the `open` method returns) or later.
 * The stream is closed by calling the `close` method. It will also be closed
 * automatically if a fatal error occurs (e. g. a TCP socket being closed by the
 * remote host).
 *
 * State diagram:
 *   .---------.
 *   | Closed  |<-----------------.
 *   '---------'                  |
 *        | open()                |
 *        V                       |
 *   .---------. Failure          |
 *   | Opening |------------------|
 *   '---------'                  |
 *        | Success               |
 *        V                       |
 *   .---------. Error or close() |
 *   | Opening |------------------'
 *   '---------'
 *
 * Whenever the state changes, the stateChanged signal is emitted. The current
 * state can also be queried using the getState method. Note, however, that the
 * state may change autonomously in response to a connection event (e. g. a TCP
 * connection being terminated) and a signal may be delayed in the event queue.
 * Therefore, when receiving a stateChanged signal, the getState method may
 * actually return a different state than the one which was passed in the
 * stateChanged event. This is particularly true if the DataStream is running on
 * a background thread.
 *
 * Note that there is currently no way to distinguish between the stream being
 * closed by the user (by calling close()) or in response to an error. The user
 * has to keep track of whether he wants the stream closed himself.
 *
 * When data is received by the stream, the dataReceived signal will be emitted.
 * There are currently no provisions for transmitting data.
 *
 * A wrapper that runs a DataStream on a background thread is available in
 * BackgroundDataStream. A wrapper that automatically reconnects a DataStream
 * and monitors data flow is available in ManagedDataStream.
 *
 * Implementations should document whether they block on opening or return
 * immediately.
 *
 * All methods of this class are thread safe. Implementations must also be
 * thread safe. In particular, this includes calling the openStream and
 * closeStream methods from arbitrary threads.
 *
 * Some methods access the back-end. While all methods are thread safe, these
 * methods may be delayed by a blocking back-end operation.
 */
class DataStream: public QObject
{
	Q_OBJECT

	public:
		// Types
		enum State { closedState, openingState, openState };

		// Construction
		DataStream (QObject *parent);
		virtual ~DataStream ();

		// Public interface
		State getState () const;
		QString getErrorMessage () const;

		// State methods
		static QString stateText (State state);

	public slots:
		// Public interface
		virtual void open ();
		virtual void close ();
		virtual void setOpen (bool o);
		virtual void applyParameters ();

	signals:
		/**
		 * Emitted whenever the state changes. Note that when receiving this
		 * signal, the state may already have changed again and the
		 * corresponding signal may have been delivered yet.
		 */
		void stateChanged (DataStream::State state);

		/**
		 * Emitted whenever the stream receives data.
		 */
		void dataReceived (QByteArray data);

		/**
		 * Emitted whenever the implementation detects that the connection is
		 * now available. Can be used to trigger a reconnect after an error.
		 */
		void connectionBecameAvailable ();

	protected:
		// Implementation interface
		/**
		 * Must be defined by implementations to do whatever is necessary to
		 * open a connection to the currently configured target. Either
		 * streamOpened or streamError MUST then be called, either immediately
		 * or later.
		 *
		 * This method is allowed to block while opening the connection.
		 * Implementations should document whether or not it blocks.
		 *
		 * This method is thread safe. It accesses the back-end.
		 */
		virtual void openStream ()=0;

		/**
		 * Must be defined by implementations to do whatever is necessary to
		 * close any existing connection.
		 *
		 * This method is allowed to block while opening the connection.
		 * Implementations should document whether or not it blocks.
		 *
		 * This method is thread safe. It accesses the back-end.
		 */
		virtual void closeStream ()=0;

		/**
		 * Must be defined by implementations to determine whether the
		 * parameters are current
		 *
		 * This method is thread safe. It accesses the back-end.
		 */
		virtual bool streamParametersCurrent ()=0;

		virtual void streamOpened ();
		virtual void streamError (const QString &errorMessage);
		virtual void streamDataReceived (const QByteArray &data);
		virtual void streamConnectionBecameAvailable ();

	private:
		QRecursiveMutex *_mutex;

		State _state;
		QString _errorMessage;

		void goToState (State state);
};

#endif
