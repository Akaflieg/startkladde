#include "Waiter.h"

#include <iostream> // TODO remove

/*
 * On synchronization:
 * - t1: thread 1 (calling notify)
 * - t2: thread 2 (calling wait)
 * - flag: finished flag
 * - wc: wait condition
 *
 * Here's and issue that necessitates synchronization:
 *
 *   Sequence diagram    Comments
 *
 *   t1  flag  wc  t2    t1             t2
 *   |   .     |   |
 *   |   .     |   |
 *   |   .<----|---|                    flag checked (not finished yet)
 *   |   .     |   |
 *   |-->.     |   |     flag set
 *   `---|---->.   |     wait condition notified
 *   .   |     .   |
 *   .   |     .<--'                    wait started (never finishes)
 *   .   |     .
 */
Waiter::Waiter ():
	flag (false)
{
}

Waiter::~Waiter ()
{
}

/**
 * Lets all threads currently waiting in #wait return from #wait, and causes
 * future calls to #wait to return immediately.
 */
void Waiter::notify ()
{
	// Lock the mutex
	QMutexLocker locker (&mutex);

	// Set the finished flag and wake all thread waiting for this task.
	// This must be synchronized or it could happen between #wait checking
	// the finished flag and waiting on the finished wait condition.
	flag=true;
	waitCondition.wakeAll ();
}

/**
 * Waits until notify is called in another thread, or returns right away if
 * notify has already been called.
 */
void Waiter::wait ()
{
	// Lock the mutex
	QMutexLocker locker (&mutex);

	// Return if notify has already been called
	if (flag) return;

	// Wait until notify is called
	waitCondition.wait (&mutex);
}
