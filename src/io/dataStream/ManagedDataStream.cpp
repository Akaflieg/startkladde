#include "ManagedDataStream.h"

#include "assert.h"

#include <QDebug>
#include <QTimer>

#include "src/io/dataStream/BackgroundDataStream.h"
#include "src/util/qTimer.h"

// Implementation notes:
//   * We want to be able to assign a new DataStream instance to this class.
//     Therefore, we have to store the open/closed state (as requested by the
//     user) here rather than relying on the state of DataStream.
//
// Improvements:
//   * data monitoring is independent from the other functionality and should be
//     in a different class.


// ******************
// ** Construction **
// ******************

/**
 * Creates a ManagedDataStream instance with the given Qt parent.
 */
ManagedDataStream::ManagedDataStream (QObject *parent): QObject (parent),
	_backgroundStream (NULL),
	// Internal state
	_open            (false),
	_connectionState (ConnectionState::closed),
	_dataState       (DataState::none),
	// External state
	_state           (State::noConnection)
{
	// Create the timers. The timers will be deleted automatically by Qt.
	_dataTimer      = new QTimer (this);
	_reconnectTimer = new QTimer (this);

    // Setup the timers
	// TOOD make the intervals configurable
    _dataTimer     ->setInterval (2000); _dataTimer     ->setSingleShot (true);
    _reconnectTimer->setInterval (5000); _reconnectTimer->setSingleShot (true);

    // Connect the timers' signals
    connect (_dataTimer     , SIGNAL (timeout ()), this, SLOT (dataTimer_timeout      ()));
    connect (_reconnectTimer, SIGNAL (timeout ()), this, SLOT (reconnectTimer_timeout ()));
}

ManagedDataStream::~ManagedDataStream ()
{
	// _backgroundStream will be deleted automatically
	// _dataTimer will be deleted automatically
	// _reconnectTimer will be deleted automatically
}


// ***********
// ** State **
// ***********

/**
 * Determines the current external state of the managed data stream by
 * inspecting the various internal states.
 *
 * This method is not for public use: use getState instead.
 */
ManagedDataStream::State::Type ManagedDataStream::determineState () const
{
	// "No background stream" takes precedence over all following states.
	if (!_backgroundStream)
		return State::noConnection;

	// "Closed" takes precedence over all following states.
	if (!_open)
		return State::closed;

	// The following states are irrelevant unless the connection is open.
	switch (_connectionState)
	{
		case ConnectionState::closed  : return State::closed;
		case ConnectionState::opening : return State::opening;
		case ConnectionState::open    : break;
		case ConnectionState::error   : return State::error;
		// no default
	}

	switch (_dataState)
	{
		case DataState::none    : return State::open;
		case DataState::ok      : return State::ok;
		case DataState::timeout : return State::timeout;
		// no default
	}

	assert (!"Not supposed to happen");
	return State::closed;
}

/**
 * Goes to the correct state (as determined by determineState()) and emits a
 * signal if it changed.
 *
 * The reconnect and data timers are started or stopped, depending on the state.
 */
void ManagedDataStream::updateState ()
{
	// Calculate the new state
	State::Type newState = determineState ();

	if (newState != _state)
	{
		// Assign the new state
		_state=newState;

		// Start the reconnect timer if and only if we're in the error state.
		setReconnectTimerRunning (_state == State::error);

		// If we're in the open or OK state, watch for data timeouts.
		setTimerRunning (_dataTimer, _state == State::open || _state == State::ok);

		// Emit the state change signal
		emit stateChanged (_state);
	}
}

/**
 * Returns the current state.
 */
ManagedDataStream::State::Type ManagedDataStream::getState () const
{
	return _state;
}

/**
 * Goes to the given connection (sub)state and updates the state.
 *
 * When the `open` connection state is entered, the data state is set to `none`.
 */
void ManagedDataStream::goToConnectionState (ManagedDataStream::ConnectionState::Type connectionState)
{
	if (connectionState != _connectionState)
	{
		_connectionState=connectionState;

		// After the connection opens, no data has been received yet.
		if (_connectionState==ConnectionState::open)
			_dataState=DataState::none;

		updateState ();
	}
}

/**
 * Goes to the given connection (sub)state and updates the state.
 */
void ManagedDataStream::goToDataState (ManagedDataStream::DataState::Type dataState)
{
	if (dataState != _dataState)
	{
		_dataState=dataState;
		updateState ();
	}
}

/**
 * Returns a textual representation of the state. The result of this method is
 * not intended for user interaction and is not localized.
 */
QString ManagedDataStream::State::text (ManagedDataStream::State::Type state)
{
	switch (state)
	{
		case State::noConnection: return "no connection";
		case State::closed      : return "closed";
		case State::opening     : return "opening";
		case State::open        : return "open";
		case State::ok          : return "ok";
		case State::timeout     : return "timeout";
		case State::error       : return "error";
		// No default
	}

	return "?";
}

/**
 * Returns the last error message of the underlying data stream.
 * @return
 */
QString ManagedDataStream::getErrorMessage () const
{
	if (_backgroundStream)
		return _backgroundStream->getErrorMessage ();
	else
		return QString ();
}




// ****************************
// ** Underlying data stream **
// ****************************

/**
 * Sets the given data stream as the underlying data stream, updates the own
 * state and opens or closes the data stream, depending on the state.
 *
 * If streamOwned is true, the stream will be deleted whenever a new stream is
 * set or when this object is destroyed.
 */
void ManagedDataStream::setDataStream (DataStream *stream, bool streamOwned)
{
	// If the stream is already current, just apply the parameters
	if (_backgroundStream && _backgroundStream->getStream ()==stream)
	{
		_backgroundStream->applyParameters ();
		return;
	}

	// Delete the old background stream. This will stop the thread, disconnect
	// its signals and, if the old underlying stream was owned, delete it.
	delete _backgroundStream;
	_backgroundStream=NULL;

	if (stream)
	{
		// Create a new background stream for the new stream
		_backgroundStream=new BackgroundDataStream (this, stream, streamOwned);

		// Connect the new background stream's signals
		connect (_backgroundStream, SIGNAL (stateChanged        (DataStream::State)),
		         this             , SLOT   (stream_stateChanged (DataStream::State)));
		connect (_backgroundStream, SIGNAL (dataReceived        (QByteArray)),
		         this             , SLOT   (stream_dataReceived (QByteArray)));
		connect (_backgroundStream, SIGNAL (connectionBecameAvailable        ()),
		         this             , SLOT   (stream_connectionBecameAvailable ()));

		// We changed the underlying data stream. We must now do two things:
		// get the data stream into the proper state, and update our own state
		// to match the data stream's state.

		// Update our own state (synthesize a stream state change event)
		stream_stateChanged (_backgroundStream->getState ());

		// Update our connection state to reflect the current stream state.
		switch (_backgroundStream->getState ())
		{
			case DataStream::closedState : goToConnectionState (ConnectionState::closed ); break;
			case DataStream::openingState: goToConnectionState (ConnectionState::opening); break;
			case DataStream::openState   : goToConnectionState (ConnectionState::open   ); break;
			// no default
		}

		// Update the new stream's state. If the stream is already in the
		// correct state, nothing will happen. Otherwise, it will emit signals
		// to reflect its state changes, which will cause this ManagedDataStream
		// to change state.
		if (_open)
			_backgroundStream->open ();
		else
			_backgroundStream->close ();
	}
	else
	{
		// Nothing to do, but we may still have to update the state.
		updateState ();
	}
}

/**
 * Sets no data stream and updates the state.
 */
void ManagedDataStream::clearDataStream ()
{
	setDataStream (NULL, false);
}

/**
 * Returns the current underlying data stream.
 */
DataStream *ManagedDataStream::getDataStream () const
{
	if (_backgroundStream)
		return _backgroundStream->getStream ();
	else
		return NULL;
}

/**
 * Called when the state of the underlying data stream changes. Updates the own
 * state.
 */
void ManagedDataStream::stream_stateChanged (DataStream::State state)
{
	switch (state)
	{
		case DataStream::closedState:
			goToConnectionState (ConnectionState::error);
			break;
		case DataStream::openingState:
			goToConnectionState (ConnectionState::opening);
			break;
		case DataStream::openState:
			goToConnectionState (ConnectionState::open);
			break;
		// no default
	}
}

/**
 * Called when the underlying data stream receives data. Updates the own state
 * and emits the dataReceived signal.
 */
void ManagedDataStream::stream_dataReceived (QByteArray data)
{
	goToDataState (DataState::ok);

	// Restart the data timer. This will not be done by goToDataState if the
	// state did not actually change.
	_dataTimer->start ();

	emit dataReceived (data);
}

/**
 * Called when the underlying data stream signals that the connection became
 * available. If this managed data stream is in the error state, attempts a
 * reconnect immediately rather than waiting for the reconnect timeout.
 */
void ManagedDataStream::stream_connectionBecameAvailable ()
{
	// Trigger the reconnect timeout immediately
	if (_state==State::error)
		reconnectTimer_timeout ();
}


// *********************
// ** Opening/closing **
// *********************

/**
 * Opens the data stream. The data stream will be re-opened automatically on
 * error until the close method is called.
 */
void ManagedDataStream::open ()
{
	_open=true;

	if (_backgroundStream)
	{
		// Open the stream. When the connection succeeds or fails (now or later),
		// we will receive a signal. This may cause timer to be started.
		_backgroundStream->open ();
	}
}

/**
 * Closes the data stream.
 */
void ManagedDataStream::close ()
{
	_open=false;
	updateState ();

	// Stop the timers. Note that a timer event may still be in the event queue,
	// so timer slots may be invoked even when the connection is closed.
	_dataTimer     ->stop ();
	_reconnectTimer->stop ();

	if (_backgroundStream)
	{
		// Close the stream
		_backgroundStream->close ();
	}
}

/**
 * Depending on the parameter, calls the open or close method.
 */
void ManagedDataStream::setOpen (bool o)
{
	if (o)
		open ();
	else
		close ();
}

/**
 * Returns whether the stream is currently open.
 *
 * This refers to whether it has been requested open; even if the underlying
 * stream is currently closed due to an error (and waiting for a reconnect),
 * this method will return true. If you need to determine the actual state of
 * the underlying stream, use the getState method.
 */
bool ManagedDataStream::isOpen () const
{
	return _open;
}


// ************
// ** Timers **
// ************

/**
 * Starts or restarts the reconnect timer. After the timer expires, a reconnect
 * will be attempted (if the stream is in the error state).
 *
 * This method also calculates the reconnect time which can be retrieved by
 * using the getReconnectTime method.
 */
void ManagedDataStream::startReconnectTimer ()
{
	// Start the reconnect timer
	_reconnectTimer->start ();

	// Calculate the reconnect time: it's the current time plus the reconnect
	// timer interval.
	QTime now=QTime::currentTime ();
	_reconnectTime=now.addMSecs (_reconnectTimer->interval ());
}

/**
 * Stops the reconnect timer.
 *
 * This method also sets the reconnect time to invalid.
 */
void ManagedDataStream::stopReconnectTimer ()
{
	// Stop the reconnect timer
	_reconnectTimer->stop ();

	// Set the reconnect time to invalid
	_reconnectTime=QTime ();
}

/**
 * Calls startReconnectTimer or stopReconnectTimer, depending on the parameter.
 */
void ManagedDataStream::setReconnectTimerRunning (bool running)
{
	if (running)
		startReconnectTimer ();
	else
		stopReconnectTimer ();
}

/**
 * Returns the time of next reconnect, as determined on the last call to
 * startReconnectTimer.
 */
QTime ManagedDataStream::getReconnectTime () const
{
	return _reconnectTime;
}


// When closing the stream, the timers are stopped. Still, it is possible to
// receive a timer event while the stream is closed, in the following case:
//   * timer expires, timer event is enqueued
//   * stream is closed
//   * timer event is received
// Therefore, it is necessary to check if the state is open in the timer slots.

/**
 * Called when the reconnect timer expires. Attempts to reconnect if the stream
 * is in error state.
 */
void ManagedDataStream::reconnectTimer_timeout ()
{
	_reconnectTime=QTime ();

	// Only react to the event if we're still in the error state (we might have
	// reconnected or closed the stream in the meantime).
	if (_state != State::error)
		return;

	// This should never fail, because the state will be noConnection if there
	// is not background data stream. Check anyway for robustness.
	if (_backgroundStream)
		_backgroundStream->open ();
}

/**
 * Called when the timer for data reception expired. Updates the connection
 * state.
 */
void ManagedDataStream::dataTimer_timeout ()
{
	// We can do this regardless of state, the data state will be ignored if it
	// is irrelevant.
	goToDataState (DataState::timeout);
}
