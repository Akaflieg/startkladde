/*
 * Waiter.h
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#ifndef WAITER_H_
#define WAITER_H_

#include <QMutex>
#include <QWaitCondition>

/**
 * A wait condition which allows notification even before the wait call
 *
 * This can be used to wait for an operation running in a background thread to
 * finish. Unlike a normal wait condition, the wait call will return
 * immediately if the Waiter has been notified before.
 *
 * This class is thread safe. If multiple threads call #wait concurrently, all
 * of them will return when #notify is called. Calling #notify more than once
 * does not have any additional effect.
 */
class Waiter
{
	public:
		Waiter ();
		virtual ~Waiter ();

		void notify ();
		void wait ();

	private:
		QMutex mutex;
		QWaitCondition waitCondition;
		bool flag;
};

#endif
