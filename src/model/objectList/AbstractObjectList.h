/*
 * AbstractObjectList.h
 *
 *  Created on: Sep 1, 2009
 *      Author: Martin Herrmann
 */

#ifndef ABSTRACTOBJECTLIST_H_
#define ABSTRACTOBJECTLIST_H_

#include <QAbstractTableModel>

/**
 * A list of objects that acts as a QAbstractTableModel, emitting signals on
 * change.
 *
 * This model does not provide any data. It just serves as a container for
 * objects that is able to notify listeners of changes. For a model that
 * provides data for objects in an AbstractObjectList, use ObjectListModel.
 *
 * Note that this QObject can be a template as it defines no signals or slots.
 */
// FIXME: don't implement the methods that don't do anything useful?
// FIXME: can we still emit changes with 0 columns, or do we need a dummy column
// for that?
// FIXME: should probably be based on QAbstractListModel
template<class T> class AbstractObjectList: public QAbstractTableModel
{
	public:
		// Construction
		AbstractObjectList (QObject *parent=NULL);
		virtual ~AbstractObjectList ();

		// Access
		using QObject::parent; // Hidden by QAbstractItemModel::parent (const QModelIndex &)

		/**
		 * The number of objects in the list
		 *
		 * @return the number of objects in the list; always >=0
		 */
		virtual int size () const=0;

		/**
		 * Gets a reference to the object a a given position in the list
		 *
		 * @param index the list index; must be >=0 and <size()
		 * @return a reference to the object a the specified index
		 */
		virtual const T &at (int index) const=0;

//		virtual const T &at (const QModelIndex &index) const;

		/**
		 * Returns a list of objects in this ObjectList. This is probably slow
		 * (as all objects have to be copied) unless the ObjectList
		 * implementation is based on a QList, implicit sharing is used, and
		 * neither the returned list nor the ObjectList is modified.
		 *
		 * @return a QList consisting of the individual objects
		 */
		// FIXME do we really need this method?
		virtual QList<T> getList () const=0;

		// QAbstractTableModel methods
		virtual int rowCount (const QModelIndex &index) const;
		virtual int columnCount (const QModelIndex &index) const;
		virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
		virtual QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
		virtual QModelIndex index (int row, int column, const QModelIndex &parent=QModelIndex ()) const;
		virtual QModelIndex index (int row) const;

};

template<class T> AbstractObjectList<T>::AbstractObjectList (QObject *parent):
	QAbstractTableModel (parent)
{
}

template<class T> AbstractObjectList<T>::~AbstractObjectList ()
{
}


// *********************************
// ** QAbstractTableModel methods **
// *********************************

/**
 * The number of rows in the model - this is the number of objects in the list
 * for the root index, invalid else.
 *
 * @see QAbstractTableModel::rowCount
 */
template<class T> int AbstractObjectList<T>::rowCount (const QModelIndex &index) const
{
	if (index.isValid ()) return 0;
	return size ();
}

/**
 * The number of columns in the model - this is always 0.
 *
 * @see QAbstractTableModel::columnCount
 */
template<class T> int AbstractObjectList<T>::columnCount (const QModelIndex &index) const
{
	Q_UNUSED (index);
	return 0;
}

/**
 * The data for a given model index and role. This model provides no data, so
 * the resul is always an invalid value.
 *
 * @see QAbstractTableModel::data
 */
template<class T> QVariant AbstractObjectList<T>::data (const QModelIndex &index, int role) const
{
	Q_UNUSED (index);
	Q_UNUSED (role);
	return QVariant ();
}

/**
 * The header data for a given orientation and role. Returns the section
 * (starting at 1) for the display role of the vertical header, and invalid
 * else.
 *
 * @see QAbstractTableModel::headerDatas
 */
template<class T> QVariant AbstractObjectList<T>::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role!=Qt::DisplayRole) return QVariant ();

	if (orientation==Qt::Horizontal)
		return QVariant ();
	else
		return section+1;
}

template<class T> QModelIndex AbstractObjectList<T>::index (int row, int column, const QModelIndex &parent) const
{
	// The model is not hierarchical, so the parent must be invalid. Also, all
	// indexes are created with invalid parent.
	if (column!=0 || row<0 || row>=size () || parent.isValid ())
		return QModelIndex ();
	else
		return createIndex (row, column);
}

template<class T> QModelIndex AbstractObjectList<T>::index (int row) const
{
	return index (row, 0);
}

// Untested
//template<class T> const T &AbstractObjectList::at (const QModelIndex &index) const
//{
//	return at (index.row ());
//}

#endif
