/*
 * Improvements:
 *   - consider passing a pointer to the query instead of copying the query
 */
#include "ThreadSafeInterface.h"

#include <iostream>
#include <cassert>

#include <QSqlError>
#include <QEvent>
#include <QTimer>

#include "src/db/result/Result.h"
#include "src/concurrent/Returner.h"
#include "src/db/interface/DefaultInterface.h"
#include "src/db/result/CopiedResult.h"
#include "src/concurrent/monitor/OperationCanceledException.h"
#include "src/db/interface/exceptions/PingFailedException.h"
#include "src/i18n/notr.h"

// ******************
// ** Construction **
// ******************

/**
 *
 * @param info
 * @param readTimeout the read timeout, in milliseconds
 * @param keepaliveInterval the keepalive interval, in milliseconds; should be
 *                          shorter than the read timeout
 * @return
 */
ThreadSafeInterface::ThreadSafeInterface (const DatabaseInfo &info, int readTimeout, int keepaliveInterval):
	Interface (info),
	readTimeoutSeconds (readTimeout), keepaliveEnabled (false), keepaliveInterval (keepaliveInterval),
	interface (NULL), isOpen (false)
{
	// This must be done on the background thread
	QTimer::singleShot (0, this, SLOT (slot_createInterface ()));

#define CONNECT(definition) connect (this, SIGNAL (sig_ ## definition), this, SLOT (slot_ ## definition))
	CONNECT (setInfo   (Returner<void>      *, DatabaseInfo));
	CONNECT (open      (Returner<bool>      *));
	CONNECT (close     (Returner<void>      *));
	CONNECT (lastError (Returner<QSqlError> *));

	CONNECT (transaction (Returner<void> *));
	CONNECT (commit      (Returner<void> *));
	CONNECT (rollback    (Returner<void> *));

	CONNECT (executeQuery       (Returner<void>                    *, Query));
	CONNECT (executeQueryResult (Returner<QSharedPointer<Result> > *, Query, bool));
	CONNECT (queryHasResult     (Returner<bool>                    *, Query));
	CONNECT (ping               (Returner<void>                    *));
#undef CONNECT

	keepaliveTimer.moveToThread (&thread);
	connect (&keepaliveTimer, SIGNAL (timeout ()), this, SLOT (keepaliveTimer_timeout ()));

	moveToThread (&thread);
	thread.start ();
}

void ThreadSafeInterface::slot_createInterface ()
{
	// Note that the interface is created on the background thread

	// For connecting the signals, we need to know that it's a
	// DefaultInterface. Afterwards, we assign it to the
	// AbstractInterface *interface. TODO shouldn't the signal be declared
	// in AbstractInterface?
	DefaultInterface *defaultInterface=new DefaultInterface (getInfo (), readTimeoutSeconds);
	connect (defaultInterface, SIGNAL (databaseError (int, QString)), this, SIGNAL (databaseError (int, QString)));
	connect (defaultInterface, SIGNAL (executingQuery (Query)), this, SIGNAL (executingQuery (Query)));
	connect (defaultInterface, SIGNAL (readTimeout ()), this, SIGNAL (readTimeout ()), Qt::DirectConnection);
	connect (defaultInterface, SIGNAL (readResumed ()), this, SIGNAL (readResumed ()), Qt::DirectConnection);
	interface=defaultInterface;
}

ThreadSafeInterface::~ThreadSafeInterface ()
{
	delete interface;
	thread.quit ();

	std::cout << notr ("Waiting for interface worker thread to terminate...") << std::flush;
	if (thread.wait (1000)) std::cout << notr ("OK")      << std::endl;
	else                    std::cout << notr ("Timeout") << std::endl;
}



// ***********************
// ** Front-end methods **
// ***********************

void ThreadSafeInterface::setInfo (const DatabaseInfo &info)
{
	Returner<void> returner;
	emit sig_setInfo (&returner, info);
	returner.wait ();

	AbstractInterface::setInfo (info);
}

bool ThreadSafeInterface::open ()
{
	Returner<bool> returner;
	emit sig_open (&returner);
	return returner.returnedValue ();
}

void ThreadSafeInterface::close ()
{
	// Hack: The thread may currently be blocked in a non-responding keepalive,
	// so the event won't be delivered. TODO: keepalive should be a
	// functionality of DefaultInterface (?), and remove this.
	cancelConnection ();

	Returner<void> returner;
	emit sig_close (&returner);
	returner.wait ();
}

QSqlError ThreadSafeInterface::lastError () const
{
	Returner<QSqlError> returner;
	emit sig_lastError (&returner);
	return returner.returnedValue ();
}

void ThreadSafeInterface::transaction ()
{
	Returner<void> returner;
	emit sig_transaction (&returner);
	returner.wait ();
}

void ThreadSafeInterface::commit ()
{
	Returner<void> returner;
	emit sig_commit (&returner);
	returner.wait ();
}

void ThreadSafeInterface::rollback ()
{
	Returner<void> returner;
	emit sig_rollback (&returner);
	returner.wait ();
}

void ThreadSafeInterface::executeQuery (const Query &query)
{
	Returner<void> returner;
	emit sig_executeQuery (&returner, query);
	returner.wait ();
}

QSharedPointer<Result> ThreadSafeInterface::executeQueryResult (const Query &query, bool forwardOnly)
{
	Returner<QSharedPointer<Result> > returner;
	emit sig_executeQueryResult (&returner, query, forwardOnly);
	return returner.returnedValue ();
}

bool ThreadSafeInterface::queryHasResult (const Query &query)
{
	Returner<bool> returner;
	emit sig_queryHasResult (&returner, query);
	return returner.returnedValue ();
}

void ThreadSafeInterface::ping ()
{
	Returner<void> returner;
	emit sig_ping (&returner);
	returner.wait ();
}


// ********************
// ** Back-end slots **
// ********************

void ThreadSafeInterface::slot_setInfo (Returner<void> *returner, DatabaseInfo info)
{
	dontReturnVoidOrException (returner, interface->setInfo (info));
}

void ThreadSafeInterface::slot_open (Returner<bool> *returner)
{
	dontReturnOrException (returner, interface->open ());
	isOpen=true;
}

void ThreadSafeInterface::slot_close (Returner<void> *returner)
{
	isOpen=false;
	dontReturnVoidOrException (returner, interface->close ());
}

void ThreadSafeInterface::slot_lastError (Returner<QSqlError> *returner) const
{
	dontReturnOrException (returner, interface->lastError ());
}

void ThreadSafeInterface::slot_transaction (Returner<void> *returner)
{
	dontReturnVoidOrException (returner, interface->transaction ());
}

void ThreadSafeInterface::slot_commit (Returner<void> *returner)
{
	dontReturnVoidOrException (returner, interface->commit ());
}

void ThreadSafeInterface::slot_rollback (Returner<void> *returner)
{
	dontReturnVoidOrException (returner, interface->rollback ());
}

void ThreadSafeInterface::slot_executeQuery (Returner<void> *returner, Query query)
{
	dontReturnVoidOrException (returner, interface->executeQuery (query));
}

void ThreadSafeInterface::slot_executeQueryResult (Returner<QSharedPointer<Result> > *returner, Query query, bool forwardOnly)
{
	// Option 1: copy the DefaultResult (is it allowed to access the
	// QSqlQuery from the other thread? It seems to work.)
//		dontReturnOrException (returner, interface->executeQueryResult (query, forwardOnly));

	// Option 2: create a CopiedResult
	(void)forwardOnly;
	dontReturnOrException (returner, QSharedPointer<Result> (
		new CopiedResult (
			// When copying, we can always set forwardOnly
			*interface->executeQueryResult (query, true)
		)
	));
}

void ThreadSafeInterface::slot_queryHasResult (Returner<bool> *returner, Query query)
{
	dontReturnOrException (returner, interface->queryHasResult (query));
}

void ThreadSafeInterface::slot_ping (Returner<void> *returner)
{
	dontReturnVoidOrException (returner, interface->ping ());
}

// ************
// ** Others **
// ************

/**
 * Called directly, this method is not executed on the background thread.
 * When using as a slot, a DirectConnection should be used or the slot will
 * be invoked in the background thread where it won't be very useful.
 */
void ThreadSafeInterface::cancelConnection ()
{
	if (interface)
		interface->cancelConnection ();
}



// ****************
// ** Monitoring **
// ****************

bool ThreadSafeInterface::event (QEvent *e)
{
	bool isSignal=(e->type ()==QEvent::MetaCall);

	if (isSignal) stopKeepaliveTimer ();
	bool result=QObject::event (e);
	if (isSignal) startKeepaliveTimer ();

	return result;
}

void ThreadSafeInterface::keepaliveTimer_timeout ()
{
	keepalive ();
}

void ThreadSafeInterface::startKeepaliveTimer ()
{
	if (isOpen && keepaliveEnabled && keepaliveInterval>0)
		keepaliveTimer.start (keepaliveInterval);
}

void ThreadSafeInterface::stopKeepaliveTimer ()
{
	keepaliveTimer.stop ();
}

/*
 * This is not implemented as a slot for now because the keepalive timer
 * mechanism is not finished yet
 */
void ThreadSafeInterface::keepalive ()
{
	assert (QThread::currentThread ()==&thread);

	stopKeepaliveTimer ();

	// We're on the background thread, so we're allowed to use the interface
	// directly. We want this to happen synchronously, and this way we don't
	// have to use a returner.
	try
	{
		interface->ping ();
	}
	catch (OperationCanceledException &) {}
	catch (PingFailedException &) {}

	startKeepaliveTimer ();
}

void ThreadSafeInterface::setKeepaliveEnabled (bool enabled)
{
	keepaliveEnabled=enabled;

	// We have do do this on the correct thread, so use the slot
	if (keepaliveEnabled)
		QTimer::singleShot (0, this, SLOT (startKeepaliveTimer ()));
	else
		QTimer::singleShot (0, this, SLOT (stopKeepaliveTimer ()));
}
