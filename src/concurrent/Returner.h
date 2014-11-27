/*
 * Returner.h
 *
 *  Created on: 28.02.2010
 *      Author: Martin Herrmann
 */

#ifndef RETURNER_H_
#define RETURNER_H_

#include "src/StorableException.h"
#include "src/concurrent/Waiter.h"

/**
 * Returns the specified value, or an exception if a StorableException is
 * thrown during the evaluation of _value
 *
 * _returner is a pointer to a Returner<T>. _value must be of type T.
 *
 * Note that only exception that occur during the evaluation of _value are
 * caught. If multiple calls are required to compute _value, an implementation
 * function can be used:
 *   void foo (Returner<bool> *returner)
 *     { returnOrException (returner, fooImpl ()); }
 */
#define returnOrException(_returner, _value) do { \
	try { _returner->returnValue (_value); return; } \
	catch (StorableException &ex) { _returner->exception (ex); } \
} while (0)

/**
 * Like returnOrException, but does not actually return from the current
 * function.
 */
#define dontReturnOrException(_returner, _value) do { \
	try { _returner->returnValue (_value); } \
	catch (StorableException &ex) { _returner->exception (ex); } \
} while (0)

/**
 * Like returnOrException, but with a Returner<void>
 *
 * _returner is a pointer to a Returner<void>. _call is of any type.
 */
#define returnVoidOrException(_returner, _call) do { \
	try { _call; _returner->returnVoid (); return; } \
	catch (StorableException &ex) { _returner->exception (ex); } \
} while (0)

/**
 * Like returnVoidOrException, but does not actually return from the current
 * function.
 */
#define dontReturnVoidOrException(_returner, _call) do { \
	try { _call; _returner->returnVoid (); } \
	catch (StorableException &ex) { _returner->exception (ex); } \
} while (0)

// ******************
// ** ReturnerBase **
// ******************

/**
 * A common base class for Returner<T> and Returner<void>
 *
 * Contains everything related to waiting and exceptions.
 */
class ReturnerBase
{
	public:
		ReturnerBase ();
		virtual ~ReturnerBase ();

		void exception (const StorableException &thrownException);

		void wait ();

	protected:
		void waitAndRethrow ();
		Waiter waiter;

	private:
		StorableException *thrownException;
};


// *****************
// ** Returner<T> **
// *****************

/**
 * A class that can be used for synchronously returning a value or throwing an
 * exception to another thread
 *
 * The calling thread passes a pointer or a reference to the working thread and
 * calls #returnedValue.
 * The working thread must either call #returnValue or #exception. If
 * #returnValue is called, the #returnedValue call in the calling thread
 * returns the value passed to #returnValue in the working thread. If
 * #exception is called, the #returnedValue call in the calling thread throws
 * the exception passed to #exception in the working thread.
 *
 * The returned value is returned by copy. Specifically, it is copied on
 * #returnValue. #returnedValue returns a reference to the copy. If the value
 * is accessed after the returner is destroyed, another copy must be made by
 * the caller.
 *
 * This is safe even if #returnValue or #exception is called before
 * #returnedValue. #returnValue and #exception may only be called once (in
 * total). returnedValue may be called multiple times.
 *
 * The macro returnOrException can be used as a shortcut.
 *
 * T must be assignable. Note that the value will be copied at least once.
 */
template<typename T> class Returner: public ReturnerBase
{
	public:
		Returner (const T &initialValue=T ()): value (initialValue) {}

		void returnValue (const T &value);

		T &returnedValue ();

	private:
		T value;
};

/**
 * Stores a copy of the value passed and notifies the threads waiting in
 * #returnedValue so they will return from #returnedValue.
 *
 * @param value the value to return from #returnedValue in other threads; a
 *              copy of value is made, so it can be destroyed after this method
 *              returns
 */
template<typename T> void Returner<T>::returnValue (const T &value)
{
	this->value=value;
	waiter.notify ();
}

/**
 * Waits for another thread to call #returnValue or #exception and either
 * returns a reference to the copy of the value made by #returnValue or throws
 * the exception passed to #exception
 *
 * If #returnValue or #exception has already been called, this method returns
 * or throws immediately.
 *
 * @return the value passed to #returnValue, unless #exception has been called
 * @throw the exception passed to #exception, if #exception has been called
 */
template<typename T> T &Returner<T>::returnedValue ()
{
	waitAndRethrow ();

	// Not reached if there is an exception
	return value;
}

// ********************
// ** Returner<void> **
// ********************

/**
 * A specialized Returner<T> for T=void
 *
 * Differences to Returner<T>:
 *   - the constructor does not take an initial value
 *   - returnValue is replaced by #returnVoid
 *   - returnedValue is replaced by #wait
 */
template<> class Returner<void>: public ReturnerBase
{
	public:
		void returnVoid ();
};

#endif
