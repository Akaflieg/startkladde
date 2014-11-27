/*
 * OperationMonitorInterface.h
 *
 *  Created on: 04.03.2010
 *      Author: Martin Herrmann
 */

#ifndef OPERATIONMONITORINTERFACE_H_
#define OPERATIONMONITORINTERFACE_H_

#include <QString>

class QAtomicInt;
class OperationMonitor;


/**
 * The interface of an operation to an OperationMonitor
 *
 * This serves the following purposes:
 *   - progress reporting
 *   - status reporting
 *   - cancelation: the #canceled method can be used. As an alternative,
 *     checkCanceled can be called (this is also called by #status and
 *     #progress by default). #checkCanceled throws an OperationMontior::
 *     CanceledException if the canceled flag is set. Strictly speaking, this
 *     is a misuse of exceptions for flow control, but it only happens in the
 *     (rare) case of cancelation by the user, so usually, there is no
 *     performance hit. Even more, for operations that can be canceled by some
 *     other means, an OperationMontior::CanceledException can be thrown
 *     manually, thereby avoiding the need to pass in (and copy) an
 *     OperationMonitorInterface. An example is DefaultInterface::
 *     doExecuteQuery.
 */
class OperationMonitorInterface
{
	friend class OperationMonitor;

	public:
		// ** Constants
		static const OperationMonitorInterface null;

		// ** Construction
		~OperationMonitorInterface ();
		OperationMonitorInterface (const OperationMonitorInterface &other);
		OperationMonitorInterface &operator= (const OperationMonitorInterface &other);

		// Operation feedback
		void status (const QString &text, bool checkCanceled=true);
		void progress (int progress, int maxProgress, const QString &status=QString (), bool checkCanceled=true);
		void ended ();

		// Operation control
		bool canceled ();
		void checkCanceled ();

	private:
		OperationMonitorInterface (OperationMonitor *monitor);
		OperationMonitor *monitor;
		QAtomicInt *refCount;
};

#endif
