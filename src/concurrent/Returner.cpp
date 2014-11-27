#include "Returner.h"

/*
 * Improvements:
 *   - synchronize accesses to data (not strictly required if #returnValue and
 *     #exception are only called once)
 *   - add a warning if this is destroyed without having been waited on (mainly
 *     relevant for Returner<void> because we don't retrieve a result)
 */

ReturnerBase::ReturnerBase ():
	thrownException (NULL)
{
}

ReturnerBase::~ReturnerBase ()
{
	delete thrownException;
}

/**
 * Stores an exception which will be thrown by #waitAndRethrow (or
 * implementation methods calling that method)
 *
 * @param thrownException the exception to throw from #waitAndRethrow
 */
void ReturnerBase::exception (const StorableException &thrownException)
{
	this->thrownException=thrownException.clone ();
	waiter.notify ();
}

/**
 * Waits for the notification by a client thread and rethrows the exception, if
 * any
 */
void ReturnerBase::waitAndRethrow ()
{
	waiter.wait ();

	if (thrownException)
		thrownException->rethrow ();
}


/**
 * Waits for the notification by a client thread and rethrows the exception, if
 * any
 *
 * If the notification has already happened, this method returns or throws
 * immediately.
 *
 * @throw the exception passed to #exception, if #exception has been called
 */
void ReturnerBase::wait ()
{
	waitAndRethrow ();
}

/**
 * Notifies the threads waiting in #wait so they will return from #wait.
 */
void Returner<void>::returnVoid ()
{
	waiter.notify ();
}
