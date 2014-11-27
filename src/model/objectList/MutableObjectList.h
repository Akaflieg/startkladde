/*
 * MutableObjectList.h
 *
 *  Created on: Sep 1, 2009
 *      Author: Martin Herrmann
 */

#ifndef MUTABLEOBJECTLIST_H_
#define MUTABLEOBJECTLIST_H_

#include "AbstractObjectList.h"

#include <iostream>

// TODO: thread safety required?

/**
 * An implementation of AbstractObjectList which allows the list to be modified
 * explicitly.
 *
 * Objects stored in this container must be assignable; it is, however, possible
 * to store pointers to objects. In this case, the objects themselves do not
 * have to be assignable; pointers always are assignable.
 */
// TODO: maybe we should have a pointer based list instead of (or in addition
// to) the value based list
template<class T> class MutableObjectList: public AbstractObjectList<T>
{
	public:
//		// Untested
//		class Mutator
//		{
//			friend class MutableObjectList<T>;
//
//			public:
//				~Mutator () { update (); }
//
//				T* operator-> () { return &(list.list[index]); }
//				void update () { list.update (index); }
//
//			private:
//				Mutator (MutableObjectList<T> &list, int index):
//					list (list), index (index) {}
//
//				MutableObjectList<T> &list;
//				int index;
//		};
//
//		friend class Mutator;

		// Required for gcc 4.7 two-phase lookup
		using AbstractObjectList<T>::setParent;

		// Construction
		MutableObjectList (QObject *parent=NULL);
		MutableObjectList (const QList<T> &list, QObject *parent=NULL);
		MutableObjectList (const MutableObjectList<T> &other);
		MutableObjectList<T> &operator= (const MutableObjectList<T> &other);
		MutableObjectList<T> operator+ (const MutableObjectList<T> &other);
		virtual ~MutableObjectList ();


		// Object access
		// FIXME return new index?
		// FIXME allow modifying
		virtual void removeAt (int index);
		virtual void append (const T &object);
		virtual void prepend (const T &object);
		virtual void insert (int index, const T &object);
		virtual void replace (int index, const T &object);
		virtual void clear ();
		virtual void replaceList (const QList<T> &newList);
		virtual void update (int index);
//		virtual Mutator mutate (int index);
//		virtual void appendList (const QList<T> &other);
//		virtual void appendList (const AbstractObjectList<T> &other);


		// AbstractObjectList methods
		virtual int size () const;
		virtual const T &at (int index) const;
		virtual QList<T> getList () const;

	protected:
		QList<T> list;
};


// ******************
// ** Construction **
// ******************

/**
 * Creates an empty MutableObjectList
 *
 * @param parent the Qt parent
 */
template<class T> MutableObjectList<T>::MutableObjectList (QObject *parent):
	AbstractObjectList<T> (parent)
{
}

/**
 * Creates a MutableObjectList, containing the members of the specified list.
 * The same notes about copying apply as for replaceList.
 *
 * @param list the initial list of objects
 * @param parent the Qt parent
 */
template<class T> MutableObjectList<T>::MutableObjectList (const QList<T> &list, QObject *parent):
	AbstractObjectList<T> (parent),
	list (list)
{
}

template<class T> MutableObjectList<T>::MutableObjectList (const MutableObjectList<T> &other):
	AbstractObjectList<T> (other.parent ()),
	list (other.list)
{
}

template<class T> MutableObjectList<T> &MutableObjectList<T>::operator= (const MutableObjectList<T> &other)
{
	if (&other==this) return *this; // Handle self assignment

	setParent (other.parent ());
	list=other.list;
	return *this;
}

/**
 * Uses this' parent
 *
 * @param other
 * @return
 */
template<class T> MutableObjectList<T> MutableObjectList<T>::operator+ (const MutableObjectList<T> &other)
{
	return MutableObjectList<T> (list+other.list, this->parent ());
}

template<class T> MutableObjectList<T>::~MutableObjectList ()
{
}


// *******************
// ** Object access **
// *******************

/**
 * Removes the object a a given position and emits the appropriate signals.
 *
 * @param index the index of the object to be removed.
 */
template<class T> void MutableObjectList<T>::removeAt (int index)
{
	QAbstractItemModel::beginRemoveRows (QModelIndex (), index, index);
	list.removeAt (index);
	QAbstractItemModel::endRemoveRows ();
}

/**
 * Appends an object to the list and emits the appropriate signals.
 *
 * @param object the object to append. A copy of the object will be made.
 */
template<class T> void MutableObjectList<T>::append (const T &object)
{
	QAbstractItemModel::beginInsertRows (QModelIndex (), list.size (), list.size ());
	list.append (object);
	QAbstractItemModel::endInsertRows ();
}

/**
 * Prepends an object to the list and emits the appropriate signals.
 *
 * @param object the object to prepend. A copy of the object will be made.
 */
template<class T> void MutableObjectList<T>::prepend (const T &object)
{
	QAbstractItemModel::beginInsertRows (QModelIndex (), 0, 0);
	list.prepend (object);
	QAbstractItemModel::endInsertRows ();
}

/**
 * Inserts an object into the list at a given position and emits the
 * appropriate signals.
 *
 * @param index the index of the object in front of which to insert the object
 * @param object the object to insert. A copy of the object will be made.
 * @see QList::insert
 */
template<class T> void MutableObjectList<T>::insert (int index, const T &object)
{
	QAbstractItemModel::beginInsertRows (QModelIndex (), index, index);
	list.insert (index, object);
	QAbstractItemModel::endInsertRows ();
}

/**
 * Replaces the object at a given position and emits the appropriate signals.
 *
 * @param index the index of the object to replace
 * @param object the object to replace the object at the specified index with
 */
template<class T> void MutableObjectList<T>::replace (int index, const T &object)
{
	list.replace (index, object);
	update (index);
}

/**
 * Clears the list and emits the appropriate signals.
 */
template<class T> void MutableObjectList<T>::clear ()
{
	// TODO Qt 4.6: use beginResetModel and endResetModel
	list.clear ();
	QAbstractItemModel::reset ();
}

/**
 * Replaces all objects of the list with the given list and emits the
 * appropriate signals.
 *
 * A copy of the list is made, so subsequent changes to the list passed will not
 * affect the elements of this. The objects themselves are not copied due to
 * implicit sharing, so they copying is fast, as long as the passed list is not
 * modified and this list is not modified as long as the passed list exists.
 *
 *  * @param newList a list of objects to replace the list with
 */
template<class T> void MutableObjectList<T>::replaceList (const QList<T> &newList)
{
	// TODO Qt 4.6: use beginResetModel and endResetModel
	list=newList;
	QAbstractItemModel::reset ();
}

/**
 * Emits the appropriate signals to notify listeners of an object change
 *
 * While it is not possible to modify an object directly, the object may be or
 * contain a pointer to another object, which can be modified. Call this method
 * after modifying an object in the list in any way.
 *
 * @param index the index of the object that changed
 */
template<class T> void MutableObjectList<T>::update (int index)
{
	QAbstractItemModel::dataChanged (QAbstractItemModel::createIndex (index, 0), QAbstractItemModel::createIndex (index, 0));
}

//template<class T> typename MutableObjectList<T>::Mutator MutableObjectList<T>::mutate (int index)
//{
//	return Mutator (*this, index);
//}


///**
// * This is probably slow as a copy of the lists has to be made.
// */
//template<class T> void MutableObjectList<T>::appendList (const QList<T> &other)
//{
//	// TODO Qt 4.6: use beginResetModel and endResetModel
//	list+=other;
//	QAbstractItemModel::reset ();
//}

///**
// * This is probably slow as a copy of the lists has to be made.
// */
//template<class T> void MutableObjectList<T>::appendList (const AbstractObjectList<T> &other)
//{
//	// TODO Qt 4.6: use beginResetModel and endResetModel
//	list+=other.getList ();
//	QAbstractItemModel::reset ();
//}


// ********************************
// ** AbstractObjectList methods **
// ********************************

/**
 * @see AbstractObjectList::size
 */
template<class T> int MutableObjectList<T>::size () const
{
	return list.size ();
}

/**
 * @see AbstractObjectList::at
 */
template<class T> const T &MutableObjectList<T>::at (int index) const
{
	return list.at (index);
}

/**
 * Makes a copy of the list, which is fast due to Qt implicit sharing as long
 * as the returned list and this list is not modified.
 *
 * @return a QList containing all the objects of this list
 */
template<class T> QList<T> MutableObjectList<T>::getList () const
{
	return list;
}

#endif
