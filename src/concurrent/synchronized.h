/*
 * synchronized.h
 *
 *  Created on: 27.02.2010
 *      Author: Martin Herrmann
 */

#ifndef SYNCHRONIZED_H_
#define SYNCHRONIZED_H_

#include <QMutex>
#include <QMutexLocker>

#define synchronized(mutex) for (Synchronizer _ ## mutex ## _sync_ (&(mutex)); !_ ## mutex ## _sync_.done; _ ## mutex ## _sync_.done=true)

/**
 * This can be used instead of synchronized (mutex) return value; the compiler
 * will recognize that this will always return.
 */
#define synchronizedReturn(mutex, value) do { QMutexLocker _ ## mutex ## _locker_ (&mutex); return (value); } while (0)

/**
 * A helper class to be used with the synchronized macro
 *
 * Use:
 *   QMutex mutex;
 *   synchronized (mutex) { ... }
 *
 * This is equivalent to, but more concise than:
 *   QMutex mutex;
 *   {
 *     QMutexLocker locker (&mutex);
 *     ...
 *   }
 *
 * The mutex will be unlocked even if the block returns or throws an exception.
 *
 * For single statements, this can be written as:
 *   synchronized (mutex) foo ();
 *
 * Note, however, that a return in the block will not be recognized as always
 * executed, so the compiler may warn about control reaching the end of the
 * function.
 */
class Synchronizer: QMutexLocker
{
	public:
        Synchronizer (QMutex *mutex);
        Synchronizer (QRecursiveMutex *mutex);
		virtual ~Synchronizer ();

		bool done;
};

#endif /* SYNCHRONIZED_H_ */
