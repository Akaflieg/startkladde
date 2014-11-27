/*
 * EntityList.h
 *
 *  Created on: Aug 29, 2009
 *      Author: Martin Herrmann
 */

#ifndef ENTITYLIST_H_
#define ENTITYLIST_H_

#include "MutableObjectList.h"

/**
 * A subclass of MutableObjectList which contains Entities and allows access based
 * on the entity id.
 *
 * T must provide a getId method.
 *
 * Note that for a typical use, there will not be two elements with the same ID
 * in the list - but EntityList does not require that.
 *
 * @see Entity::getId
 */
template<class T> class EntityList: public MutableObjectList<T>
{
	public:
		// Required for gcc 4.7 two-phase lookup
		using MutableObjectList<T>::replace;
		using MutableObjectList<T>::append;

		// Construction
		EntityList (QObject *parent=NULL);
		EntityList (const QList<T> &list, QObject *parent=NULL);
		virtual ~EntityList ();
		EntityList<T> operator+ (const EntityList<T> &other);

		// Access
		// TODO: addById which gets the data from the cache (?); dito for replace
		virtual int findById (dbId id);
		// TODO: have them return the number of entries removed/replaced and
		// assert that it is 1 where applicable
		virtual void removeById (dbId id);
		// TODO: replace method which only takes the object and reads the ID from the object
		virtual void replaceById (dbId id, const T &object);
		virtual void replaceOrAdd (dbId id, const T &object);
};


// ******************
// ** Construction **
// ******************

template<class T> EntityList<T>::EntityList (QObject *parent):
	MutableObjectList<T> (parent)
{
}

template<class T> EntityList<T>::EntityList (const QList<T> &list, QObject *parent):
	MutableObjectList<T> (list, parent)
{
}

template<class T> EntityList<T>::~EntityList ()
{
}

/**
 * Uses this' parent
 *
 * @param other
 * @return
 */
template<class T> EntityList<T> EntityList<T>::operator+ (const EntityList<T> &other)
{
	return EntityList<T> (this->list+other.list, this->parent ());
}


// ************
// ** Access **
// ************

/**
 * Finds and returns the index of one element with the given ID. If the list
 * contains no object with the given ID, -1 is returned.
 *
 * @param id the ID to look for
 * @return the index of an object with the specified ID, or -1
 */
template<class T> int EntityList<T>::findById (dbId id)
{
	// TODO cache in a map?
	for (int i=0; i<MutableObjectList<T>::size (); ++i)
		if (MutableObjectList<T>::at (i).getId ()==id)
			return i;

	return -1;
}

/**
 * Removes all elements with the given ID.
 *
 * @param id the ID of the object(s) to remove
 */
template<class T> void EntityList<T>::removeById (dbId id)
{
	// TODO cache in a map?
	// TODO use iterator?
	for (int i=0; i<MutableObjectList<T>::size (); ++i)
	{
		if (MutableObjectList<T>::at (i).getId ()==id)
		{
			MutableObjectList<T>::removeAt (i);
			--i;
		}
	}
}

/**
 * Replaces all elements that have the given ID with the given object.
 *
 * @param id the ID of the object(s) to replace
 * @param object the object to replace said objects with
 */
template<class T> void EntityList<T>::replaceById (dbId id, const T &object)
{
	// TODO cache in a map?
	for (int i=0; i<MutableObjectList<T>::size (); ++i)
		if (MutableObjectList<T>::at (i).getId ()==id)
			replace (i, object);
}

/**
 * Replaces all objects that have the given ID with a given object; if no
 * object in the list has the given ID, the new object is appended to the list.
 *
 * @param id the ID of the object(s) to replace
 * @param object the new object
 */
template<class T> void EntityList<T>::replaceOrAdd (dbId id, const T &object)
{
	int index=findById (id);

	if (index>=0)
		replaceById (id, object);
	else
		append (object);
}

#endif
