#ifndef MAYBE_H_
#define MAYBE_H_

#include <cassert>

#include <QList>

/**
 * A container that may or may not contain a value of a given type
 *
 * A Maybe is considered valid if it contains a value, or invalid else.
 *
 * A Maybe essentially behaves like a QList, except that it is limited to one
 * value. Modifying a Maybe will not affect any copies of that Maybe.
 *
 * A Maybe stores a value, but the container is implicitly shared, so copying
 * is very fast as long as the value is not modified.
 *
 * Note that it is possible to pass a pointer as well as a reference to the
 * constructor, so we can create an empty Maybe by passing NULL. However, this
 * means that using a Maybe with a pointer type may lead to undefined behavior.
 * In particular, when calling the constructor with NULL, it is unclear whether
 * the resulting Maybe will be invalid or valid containing the value NULL.
 * Therefore, a Maybe should not be used to store a pointer. Since pointers can
 * already be NULL to indicate "no value", this should not be a serious
 * restriction.
 */
template<typename T> class Maybe
{
	public:
		// Construction
		Maybe ();
		Maybe (const T &value);
		Maybe (const T *value);
		static Maybe<T> invalid ();
		virtual ~Maybe ();

		// Read access
		bool isValid () const;
		T &getValue ();
		T *operator-> ();
		const T &getValue () const;
		const T *operator-> () const;

		// Write access
		void setValue (const T &value);
		void clearValue ();

	private:
		// This is implemented using QList to get free copy constructor,
		// assignment operator and implicit sharing
		QList<T> list;
};


// ******************
// ** Construction **
// ******************

/**
 * Constructs a valid Maybe with the specified value
 *
 * The value is copied.
 */
template<typename T> Maybe<T>::Maybe (const T &value)
{
	list.append (value);
}

/**
 * Constructs an invalid or a valid Maybe with the specified value
 *
 * If the value is NULL, the Maybe will be invalid. Otherwise, the value will
 * be copied.
 */
template<typename T> Maybe<T>::Maybe (const T *value)
{
	if (value)
		list.append (*value);
}

/**
 * Constructs an invalid Maybe
 *
 * Consider using Maybe<T>::invalid () instead for more clarity.
 */
template<typename T> Maybe<T>::Maybe ()
{
}

/**
 * Creates an invalid Maybe
 */
template<typename T> Maybe<T> Maybe<T>::invalid ()
{
	return Maybe<T> ();
}

template<typename T> Maybe<T>::~Maybe ()
{
}


// *****************
// ** Read access **
// *****************

/**
 * Returns true if this Maybe is valid, false if not
 */
template<typename T> bool Maybe<T>::isValid () const
{
	return (!list.isEmpty ());
}

/**
 * Returns a reference to the value
 *
 * This method may only be called if this Maybe is valid. The returned reference
 * gets invalid if the Maybe is changed.
 */
template<typename T> T &Maybe<T>::getValue ()
{
	// This will assert if the value is not valid
	return list[0];
}

/**
 * Returns a pointer to the value so the -> operator can be used directly on the
 * Maybe.
 */
template<typename T> T *Maybe<T>::operator-> ()
{
	// This will assert if the value is not valid
	return &(list[0]);
}

/**
 * Same as getValue, but returns a constant reference
 */
template<typename T> const T &Maybe<T>::getValue () const
{
	// This will assert if the value is not valid
	return list.at (0);
}

/**
 * Same as operator*, but returns a constant pointer
 */
template<typename T> const T *Maybe<T>::operator-> () const
{
	// This will assert if the value is not valid
	return &(list.at (0));
}


// ******************
// ** Write access **
// ******************

template<typename T> void Maybe<T>::setValue (const T &value)
{
	list.clear ();
	list.append (value);
}

template<typename T> void Maybe<T>::clearValue ()
{
	list.clear ();
}


#endif
