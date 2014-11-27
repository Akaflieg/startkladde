#ifndef SORTEDSET_IMPL_H_
#define SORTEDSET_IMPL_H_

#include "SortedSet.h"

/*
 * This file contains the implementations of the template SortedSet. It must be
 * included in all files that call SortedSet methods. Files that only declare
 * SortedSet instances should only include SortedSet.h.
 * The implementation in this file is separate from the class declaration in
 * SortedSet.h to reduce compile time dependencies, e. g. from MainWindow.cpp
 * via Cache.h, by only including the implementation in Cache.cpp.
 */

// *****************
// ** Data access **
// *****************

/**
 * Removes all items from the set
 *
 * @return true if anything was changed
 */
template<typename T> bool SortedSet<T>::clear ()
{
	if (data.isEmpty ()) return false;

	data.clear ();
	invalidateList ();
	return true;
}

/**
 * Determines whether the item contains the given value
 *
 * @param value a value
 * @return true if the set contains value, false if not
 */
template<typename T> bool SortedSet<T>::contains (const T &value) const
{
	return data.contains (value);
}

/**
 * Inserts the given value into the set, unless it is already present
 *
 * @param value the value to insert
 * @return true if the value was inserted, false if it was already present
 */
template<typename T> bool SortedSet<T>::insert (const T &value)
{
	if (data.contains (value)) return false;

	data.insert (value, 0);
	invalidateList ();
	return true;
}

/**
 * Determines whether the set is empty
 *
 * @return true if the set is empty, false if not
 */
template<typename T> bool SortedSet<T>::isEmpty () const
{
	return data.isEmpty ();
}

/**
 * Removes an entry from the set if it is present
 *
 * @param value the value to remove
 * @return true if the value was present, false if not
 */
template<typename T> bool SortedSet<T>::remove (const T &value)
{
	if (data.remove (value)>0)
	{
		invalidateList ();
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Determines the number of elements in the set
 *
 * @return the number of elements in the set
 */
template<typename T> int SortedSet<T>::size () const
{
	return data.count ();
}


// **********
// ** List **
// **********

/**
 * Returns a QList containing all values from the set, in order
 *
 * The QList is cached, so due to Qt's implicit sharing, it is very fast as
 * long as the set is not changed. If the set is changed, the list will be
 * regnerated on the next access.
 */
template<typename T> QList<T> SortedSet<T>::toQList () const
{
	return generateList ();
}

template<typename T> SortedSet<T> &SortedSet<T>::operator= (const QList<T> &list)
{
	// No need to handle self assignment - different type

	data.clear ();

	foreach (const T &value, list)
		data.insert (value, 0);

	invalidateList ();

	return *this;
}

/**
 * Marks the list as invalid, so it will be regenerated on the next access
 */
template<typename T> void SortedSet<T>::invalidateList ()
{
	listValid=false;
}

/**
 * Regenerates a list if necessary
 *
 * @return a reference to the list
 */
template<typename T> QList<T> &SortedSet<T>::generateList () const
{
	if (!listValid)
	{
		generatedList=data.keys ();
		listValid=true;
	}

	return generatedList;
}

#endif
