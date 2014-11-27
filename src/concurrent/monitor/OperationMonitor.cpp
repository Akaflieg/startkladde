#include "OperationMonitor.h"

#include "src/concurrent/synchronized.h"

/*
 * Improvements:
 *   - allow suboperations
 *   - better progress reporting (e. g. with foreach loop): maxProgress() and
 *     next()
 *   - integrate Monitor with Returner so we only need to pass one object
 *     - NB: returner is a template, and we want to emit (progress, status) and
 *       receive (cancel) signals
 *     - Maybe use a Listener
 *     - Maybe use a QVariant
 *     - Look at QFuture
 *   - make Returner assignable so we can write Retuner returner=operation ();
 *     and return operation ().returnedValue (); (?)
 *   - ThreadSafeInterface open vs. asyncOpen: could have this for all methods
 *     -> merge somehow? (e. g. pass Monitor::Synchronous ())
 *   - Use a monitor with a method that does not take a monitor?
 *   - The block for calling a background operation is quite long. Better would
 *     be something like:
 *     Monitor<T> monitor=asyncOperation (params...);
 *     monitor.getResult (); // waits and rethrows
 *     monitor.wait (); // rethrows
 *     For specifying different kinds of monitors, we'll probably need
 *     asyncOperation (new SignalMonitor<T> (), params...);
 *   - Allow canceling before the operation has started; but beware the race
 *     condition that may occur if the canceled flag is set right after it has
 *     been reset and thus prematurely cancels the next operation
 *   - Better integration with cancelConnection of a proxy; currently, the
 *     class using the proxy has to be connected manually
 *   - OperationMonitor should be copied so we can call a method with a monitor
 *     and then discard the monitor instance
 *   - Storing and getting of status, progress etc. should be in the base class
 *     (or at least a DefaultOperationMonitor)
 *   - Can we have an "AsynchronousInterface" which inherites Interface and
 *     stores a Monitor? What about return values?
 *   - The connection canceling should be in the interface, not in the caller
 *   - Warning if an operation tries to use a monitor which is already in use
 *   - Indicate if the operation has not started yet (because another operation
 *     is currently performed by that thread)
 */

OperationMonitor::OperationMonitor ():
	theInterface (this), canceled (false)
{
}

OperationMonitor::~OperationMonitor ()
{
}

/**
 * Returns a copy of the interface of the operation monitor
 *
 * When the last copy of the interface is destroyed, the end of the operation
 * is signaled to the monitor by the interface.
 *
 * @return a copy of the interface
 */
OperationMonitorInterface OperationMonitor::interface ()
{
	// Make a copy of the interface
	return theInterface;
}

/**
 * Cancels the operation
 *
 * The operation must support cancelation by regularly checking the canceled
 * flag or calling checkCanceled (potentially indirectly through one of the
 * status/progress methods). If the operation does not support canelation, this
 * has no effect. The operation may not be canceled immediately. Wait for the
 * ended notification.
 */
void OperationMonitor::cancel ()
{
	synchronized (mutex)
		canceled=true;
}

/**
 * Determines whether the operation has been canceled.
 *
 * @return true if the operation has been canceled, false if not
 */
bool OperationMonitor::isCanceled ()
{
	synchronizedReturn (mutex, canceled);
}
