#include "DataStream.h"

#include <QDebug>
#include <QMutex>
#include <QMutexLocker>

#include "src/i18n/notr.h"
#include "src/concurrent/DefaultQThread.h"

/*
 * Architecture rationale:
 *
 * We can't simply put the back-end class (e. g. QIODevice implementation) on
 * a background thread, because:
 *   - setup/open is not thread safe
 *   - open isn't a slot (setup neither)
 *
 * It may not be allowed to access the back-end in a thread other than where it
 * was created. Therefore, the whole DataStream must live on a background thread
 * where it creates, opens and accesses the back-end. It must therefore be
 * thread safe.
 *
 * Potential improvements:
 *   - Less code in background thread: only have back-end on background thread,
 *     not the state tracking
 *   - We can't call streamParametersCurrent() from the GUI thread as it
 *     accesses the back-end.
 *   - We shouldn't abuse the connectionBecameAvailable mechanism for
 *     reconnecting on settings change when in error state
 */

// ******************
// ** Construction **
// ******************

/**
 * Creates a new DataStream instance with the specified Qt parent
 */
DataStream::DataStream (QObject *parent): QObject (parent),
    _mutex (new QRecursiveMutex ()),
	_state (closedState)

{
	// The stream is initially in the closed state.
}

DataStream::~DataStream ()
{
	delete _mutex;
}


// ********************
// ** User interface **
// ********************

/**
 * Opens the stream.
 *
 * If the stream is already opening or open, nothing happens. Otherwise, the
 * stream will go to the `opening` state. Depending on the implementation, it
 * may go to the `open` (on success) or `closed` (on failure) state immediately
 * before this method returns, or this method may return and the stream will go
 * to the `open` or `closed` state later.
 *
 * After calling this method, the state of the stream can be `opening` (if
 * opening is delayed, e. g. waiting for the remote side to accept the
 * connection), `open` (if the stream opened immediately) or `closed` (if the
 * connection failed immediately).
 *
 * This method is thread safe. It accesses the back-end.
 */
void DataStream::open ()
{
	// Note that the
	QMutexLocker locker (_mutex);

	// If the stream is already open or opening, there's nothing to do.
	if (_state!=closedState)
		return;

	// Update the state
	goToState (openingState);

	// Open the stream. When the operation succeeds (now or later),
	// streamOpenSuccess will be called. When the operation fails (now or
	// later), streamOpenFailure will be called. When the error is closed due to
	// an error (after it has been opened), streamError will be called.
	// The openStream method may block, so we have to unlock the mutex before
	// the call.
	locker.unlock ();
	openStream ();
}

/**
 * Closes the stream.
 *
 * If the stream is already closed, nothing happens. Otherwise, it will go to
 * the `closed` state before this method returns.
 *
 * This method is thread safe. It accesses the back-end.
 */
void DataStream::close ()
{
	QMutexLocker locker (_mutex);

	// If the stream is already closed, there's nothing to do.
	if (_state==closedState)
		return;

	// Update the state
	goToState (closedState);

	// Close the stream
	// The openStream method may block, so we have to unlock the mutex before
	// the call.
	locker.unlock ();
	closeStream ();
}

/**
 * Calls either `open` (if the parameter is `true`) or `close` (otherwise).
 *
 * This method (slot) can be useful to connect to a signal of, e. g., a QAction
 * that controls opening of the stream.
 *
 * This method is thread safe. It accesses the back-end.
 */
void DataStream::setOpen (bool o)
{
	// Locking is not necessary as we do not access any data members and only
	// call thread safe methods.
	if (o)
		open ();
	else
		close ();
}

/**
 * Closes and re-opens the stream if the parameters are outdated.
 *
 * This method is thread safe. It accesses the back-end.
 */
void DataStream::applyParameters ()
{
	QMutexLocker locker (_mutex);
	State state=_state;
	locker.unlock ();

	if (!streamParametersCurrent ())
	{
		if (state==closedState)
		{
			// Suggest that we may try to connect right now
			// TODO the decisions about connecting (closed state) and
			// reconnecting (other states) should be made in the same place,
			// probably in ManagedDataStream.
			emit connectionBecameAvailable ();
		}
		else
		{
			// Note that we call close() and open() rather than closeStram()
			// and openStream() so we get the proper state changes.
			close ();
			open ();
		}
	}
}


/**
 * Returns the current state of the data stream.
 *
 * This method is thread safe. It does not access the back-end.
 */
DataStream::State DataStream::getState () const
{
	QMutexLocker locker (_mutex);
	return _state;
}

/**
 * Returns the last error message.
 *
 * This method is thread safe. It does not access the back-end.
 */
QString DataStream::getErrorMessage () const
{
	QMutexLocker locker (_mutex);
	return _errorMessage;
}


// ***********************
// ** Private interface **
// ***********************

/**
 * Goes to the specified state.
 *
 * This method stores the specified state and emits a stateChanged signal.
 *
 * This method is thread safe. It does not access the back-end.
 */
void DataStream::goToState (DataStream::State state)
{
	// This method may be called from other thread safe methods holding the
	// mutex. Since the mutex is recursive, we can still lock it here.
	QMutexLocker locker (_mutex);

	_state=state;
	emit stateChanged (state);
}


// ******************************
// ** Implementation interface **
// ******************************

/**
 * Called by implementations whenever the stream is successfully opened.
 *
 * This method is thread safe. It does not access the back-end.
 */
void DataStream::streamOpened ()
{
	QMutexLocker locker (_mutex);

	// The stream implementation may be opened with a delay (e. g. when using
	// QTcpSocket), so the stream may have been closed in the meantime. In this
	// case, ignore the opened event.
	if (_state==openingState)
		goToState (openState);
}

/**
 * Called by implementations whenever the stream fails to open or experiences
 * a fatal error.
 *
 * Implementations should make sure that the underlying mechanism is closed and
 * ready to be re-opened before calling this method.
 *
 * This method is thread safe. It does not access the back-end.
 */
void DataStream::streamError (const QString &errorMessage)
{
	QMutexLocker locker (_mutex);

	// The stream may have been closed in the meantime. In this case, ignore the
	// error.
	if (_state!=closedState)
	{
		_errorMessage=errorMessage;
		goToState (closedState);
	}
}

/**
 * Called by implementations when data is received from the stream.
 *
 * This method is thread safe. It does not access the back-end.
 */
void DataStream::streamDataReceived (const QByteArray &data)
{
	QMutexLocker locker (_mutex);

	//qDebug () << data.trimmed ();

	// The stream may have been closed in the meantime. In this case, ignore the
	// data.
	if (_state==openState)
		emit dataReceived (data);
}

/**
 * Can be called by implementations when it seems likely that the connection can
 * now be established when it couldn't before; e. g. when the required hardware
 * is plugged in.
 *
 * The corresponding signal may, for example, be used to initiate a reconnect
 * the connection failed.
 *
 * Implementations are not required to support this mechanism.
 *
 * This method is thread safe. It does not access the back-end.
 */
void DataStream::streamConnectionBecameAvailable ()
{
	// This method does not access any properties and does not call any methods
	// (except signals). It is therefore thread safe.

	// It seems to be possible (observed on a Windows 7 system) that opening a
	// serial port immediately after it becomes available fails with a "file not
	// found" error (ERROR_FILE_NOT_FOUND). This delay seems to work around the
	// issue.
	DefaultQThread::msleep (500);

	emit connectionBecameAvailable ();
}


// ***********
// ** State **
// ***********

/**
 * Returns a readable representation of the state.
 *
 * This method can be useful for logging, but the return value should probably
 * not be shown to the user, since the text is not localized and may change in
 * the future.
 *
 * This method is thread safe. It does not access the back-end.
 */
QString DataStream::stateText (DataStream::State state) // static
{
	// This method does not access any properties and does not call any methods.
	// It is therefore inherently thread safe.

	switch (state)
	{
		case closedState : return notr ("closed" );
		case openingState: return notr ("opening");
		case openState   : return notr ("open"   );
		// no default
	}

	return notr ("?");
}
