/*
 * StorableException.h
 *
 *  Created on: 28.02.2010
 *      Author: Martin Herrmann
 */

#ifndef STORABLEEXCEPTION_H_
#define STORABLEEXCEPTION_H_

/**
 * An exception which can be caught, stored, and later rethrown without knowing
 * the precise type of the exception
 *
 * This is achieved by providing two methods, #clone and #rethrow, which
 * every subclass must reimplement (even if derived from a class implementing
 * the methods).
 *
 * The exception is caught by reference. A copy ist made using #clone and the
 * pointer is stored. Later, the stored exception can be rethrown by calling
 * #rethrow.
 *
 * #rethrow will typically be called via a pointer on an instance on the heap
 * previously created by #clone (). Note that in order to delete the instance,
 * you will have to use the RAII pattern as #rethrow method does not return.
 *
 * An example:
 *   QSharedPointer<StorableException> storedException; // RAII pattern
 *
 *   try
 *   {
 *     // ...
 *   }
 *   catch (StorableException &ex)
 *   {
 *     storedException=QSharedPointer<StorableException> (ex.clone ());
 *   }
 *
 *   // ...
 *
 *   if (storedException)
 *     storedException->rethrow ();
 *
 * The Returner class uses StorableExceptions to throw exceptions across thread
 * boundaries.
 */
/*
 * This solution was inspired by http://stackoverflow.com/questions/667077/
 * c-cross-thread-exception-handling-problem-with-boostexception
 */
class StorableException
{
	public:
		virtual ~StorableException ();

		/**
		 * Creates a copy of this exception
		 *
		 * This method must be reimplemented by every subclass, even if
		 * inherited from a class implementing it.
		 *
		 * @return a new instance of the same actual class; the caller takes
		 *         ownership of the result
		 */
		virtual StorableException *clone () const=0;

		/**
		 * Throws a copy of this exception
		 *
		 * This method must be reimplemented by every subclass, even if
		 * inherited from a class implementing it.
		 *
		 * This method does not return.
		 *
		 * @throw itself
		 */
		virtual void rethrow () const=0;
};

#endif /* STORABLEEXCEPTION_H_ */
