/*
 * ObjectListModel.h
 *
 *  Created on: Aug 18, 2009
 *      Author: Martin Herrmann
 */

#ifndef OBJECTLISTMODEL_H_
#define OBJECTLISTMODEL_H_

#include <iostream>

#include <QAbstractTableModel>

#include "AbstractObjectList.h"

template<class T> class ObjectModel;

/*
 * Potential improvements; this note should go to the proxy model
 *   - add a default sorting column; currently, ObjectListWindow sorts by
 *     column 0 by default.
 *   - allow a custom sorting order (for example for a person: last name,
 *     first name). Note that this requires a way to specify that sort order
 *     rather than "sort by column x".
 */

// *************************
// ** ObjectListModelBase **
// *************************

/**
 * A non-template base class for ObjectListModel. Required for defining slots.
 */
class ObjectListModelBase: public QAbstractTableModel
{
	Q_OBJECT

	public:
		ObjectListModelBase (QObject *parent=NULL): QAbstractTableModel (parent) {}

	public slots:
		virtual void listDataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)=0;
		virtual void listDestroyed ()=0;
		virtual void modelDestroyed ()=0;
};

// *********************
// ** ObjectListModel **
// *********************

/**
 * A table model that gets the rows from an AbstractObjectList and the columns
 * from an ObjectModel.
 *
 * Note that this list cannot be sorted. For a sorted view, use a
 * QSortFilterProxyModel.
 */
template<class T> class ObjectListModel: public ObjectListModelBase
{
	public:
		ObjectListModel (QObject *parent);
		virtual ~ObjectListModel ();

		// Model
		void setList  (AbstractObjectList<T> *list , bool owned);
		void setModel (ObjectModel       <T> *model, bool owned);
		AbstractObjectList<T> *list  () { return _list;  }
		ObjectModel       <T> *model () { return _model; }


		// Access
		//virtual bool hasIndex (const QModelIndex &index) const;
		//virtual bool hasRow (int index) const;
		virtual const T &at (const QModelIndex &index) const;
		virtual const T &at (int row) const;

		// QAbstractTableModel methods
		virtual int rowCount (const QModelIndex &index=QModelIndex ()) const;
		virtual int columnCount (const QModelIndex &index=QModelIndex ()) const;
		virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
		virtual QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;

		// List changes
		virtual void listDataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);
		virtual void listDestroyed ();

		// Model changes
		virtual void modelDestroyed ();

		// Other changes
		virtual void refreshColumn (int column);
		virtual void refreshAll ();

		virtual int mapToSource (const QModelIndex &index);
		virtual QModelIndex mapFromSource (int sourceIndex, int column);

	protected:
		AbstractObjectList<T> *_list ; bool  _listOwned;
		ObjectModel       <T> *_model; bool _modelOwned;
};

// TODO this should provide setList and setModel

// ************************************
// ** ObjectListModel implementation **
// ************************************

/**
 * Creates an ObjectListModel. It can optionally take ownership of the list
 * and the model. If ownership is taken, the corresponding object is deleted
 * when this class is deleted. If ownership is not taken, the objects are not
 * deleted.
 *
 * @param list the object list to get the rows from
 * @param listOwned whether the ObjectListModel takes ownership of the list
 * @param model the model to get the columns from
 * @param modelOwned whether the ObjectListModel takes ownership of the model
 * @param parent the Qt parent of this object
 * @return
 */
template<class T> ObjectListModel<T>::ObjectListModel (QObject *parent):
	ObjectListModelBase (parent),
	_list  (NULL), _listOwned  (false),
	_model (NULL), _modelOwned (false)
{
}

/**
 * Deletes the model and/or list if they are owned.
 */
template<class T> ObjectListModel<T>::~ObjectListModel ()
{
	if (_modelOwned) delete _model;
	if (_listOwned)  delete _list;
}


// ***********
// ** Model **
// ***********

template<class T> void ObjectListModel<T>::setList (AbstractObjectList<T> *list, bool owned)
{
	// Nothing to do if the same list is already set
	if (_list==list)
		return;

	beginResetModel ();

	// If we own the list, delete it. If not, just disconnect its signals.
	if (_list)
	{
		if (_listOwned)
			delete _list;
		else
			_list->disconnect (this);
	}

	// Set the new list
	_list=list;
	_listOwned=owned;

	// Connect the new list's signals
#define reemitSignal(signal) do { QObject::connect (_list, SIGNAL (signal), this, SIGNAL (signal)); } while (0)
	reemitSignal (columnsAboutToBeInserted (const QModelIndex &, int, int));
	reemitSignal (columnsAboutToBeRemoved (const QModelIndex &, int, int));
	reemitSignal (columnsInserted (const QModelIndex &, int, int));
	reemitSignal (columnsRemoved (const QModelIndex &, int, int));
	reemitSignal (headerDataChanged (Qt::Orientation, int, int));
	reemitSignal (layoutAboutToBeChanged ());
	reemitSignal (layoutChanged ());
	reemitSignal (modelAboutToBeReset ());
	reemitSignal (modelReset ());
	reemitSignal (rowsAboutToBeInserted (const QModelIndex &, int, int));
	reemitSignal (rowsAboutToBeRemoved (const QModelIndex &, int, int));
	reemitSignal (rowsInserted (const QModelIndex &, int, int));
	reemitSignal (rowsRemoved (const QModelIndex &, int, int));
	connect ( // dataChanged is different, see listDataChanged
		_list, SIGNAL (dataChanged     (const QModelIndex &, const QModelIndex &)),
		this,  SLOT   (listDataChanged (const QModelIndex &, const QModelIndex &)));
	connect (_list, SIGNAL (destroyed ()), this, SLOT (listDestroyed ()));
#undef reemitSignal

	endResetModel ();
}

template<class T> void ObjectListModel<T>::setModel (ObjectModel<T> *model, bool owned)
{
	// Nothing to do if the same model is already set
	if (_model==model)
		return;

	beginResetModel ();

	// If we own the model, delete it. If not, just disconnect its signals.
	if (_model)
	{
		if (_modelOwned)
			delete _model;
		// ObjectModel does not inherit from QObject. FIXME it should
		//else
		//	_model->disconnect (this);
	}

	// Set the new model
	_model=model;
	_modelOwned=owned;

	// Connect the new model's signals
	// ObjectModel does not inherit from QObject. FIXME it should
	//connect (_model, SIGNAL (destroyed ()), this, SLOT (_modelDestroyed ()));

	endResetModel ();
}

template<class T> void ObjectListModel<T>::listDestroyed ()
{
	_list=NULL;
	// Unfortunately, we were not notified before the change, so we aren't able
	// to use beginResetModel/endResetModel as a good model should.
    beginResetModel();
    endResetModel();
}

template<class T> void ObjectListModel<T>::modelDestroyed ()
{
	_model=NULL;
	// Unfortunately, we were not notified before the change, so we aren't able
	// to use beginResetModel/endResetModel as a good model should.
    beginResetModel();
    endResetModel();
}


// *********************************
// ** QAbstractTableModel methods **
// *********************************

template<class T> int ObjectListModel<T>::rowCount (const QModelIndex &index) const
{
	if (!_list) return 0;
	if (index.isValid ()) return 0;
	return _list->size ();
}

template<class T> int ObjectListModel<T>::columnCount (const QModelIndex &index) const
{
	if (!_model) return 0;
	if (index.isValid ()) return 0;
	return _model->columnCount ();
}

template<class T> QVariant ObjectListModel<T>::data (const QModelIndex &index, int role) const
{
	if (!_model || !_list)
		return QVariant ();

	int column=index.column ();

	const T &object=_list->at (index.row ());
	return _model->data (object, column, role);
}

template<class T> QVariant ObjectListModel<T>::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (!_model)
		return QVariant ();

	if (orientation==Qt::Horizontal)
		return _model->headerData (section, role);
	else
		return section+1;
}


// **********
// ** Misc **
// **********

// Untested
//template<class T> bool ObjectListModel<T>::hasRow (int row) const
//{
//	if (!_list)
//		return false;
//
//	return (row>=0 && row<list->size ());
//}

// Untested
//template<class T> bool ObjectListModel<T>::hasIndex (const QModelIndex &index) const
//{
//	if (!_list || !_model)
//		return false;
//
//	return (
//		index.isValid () &&
//		index.row ()>=0 &&
//		index.column ()>=0 &&
//		index.row ()<list->size () &&
//		index.column ()<model->columnCount ()
//		);
//}

/**
 * Gets a reference to the object specified by a model index from the list
 *
 * @param index the index in this model of the object to be returned; only the
 *              row of the model index is used; the index must be in range
 * @return a reference to the object
 */
template<class T> const T &ObjectListModel<T>::at (const QModelIndex &index) const
{
	return _list->at (index.row ());
}

/**
 * Gets a reference to the object specified by a row number
 *
 * @param index the row of the object to be returned; the index must be in range
 * @return a reference to the object
 */
template<class T> const T &ObjectListModel<T>::at (int row) const
{
	return _list->at (row);
}

/**
 * Will be called when the contents of the list change. All other change
 * signals of AbstractObjectList refer to rows as such and can thus be
 * reemitted without change. However, the dataChanged signal refers to colums,
 * of which the AbstractObjectList only has one, but the ObjectListModel can
 * have several (as determined by the object model). Thus, we have to emit
 * the signal with different parameters.
 *
 * @param topLeft the upper left index of the data that changed; the index
 *                refers to the AbstractObjectList (which has only one column)
 * @param bottomRight the bottom right index of the data that changed; the index
 *                refers to the AbstractObjectList (which has only one column)
 */
template<class T> void ObjectListModel<T>::listDataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	// As this is only called after the data changed, we cannot determine which
	// of the columns are affected, so we emit a change of all columns
	QModelIndex newTopLeft=createIndex (topLeft.row (), 0);
	QModelIndex newBottomRight=createIndex (bottomRight.row (), columnCount ()-1);

	emit dataChanged (newTopLeft, newBottomRight);
}

/**
 * Emits a change of all values of the specified column. This may be useful to
 * refresh a column whose value depends on the time.
 *
 * @param column the number of the column; must be >=0 and <columnCount
 */
template<class T> void ObjectListModel<T>::refreshColumn (int column)
{
	QModelIndex topLeft=createIndex (0, column);
	QModelIndex bottomRight=createIndex (rowCount ()-1, column);

	emit dataChanged (topLeft, bottomRight);
}

/**
 * Emits a change of all values. This may be useful to refresh the data when
 * something fundamental changes, for example the program language.
 */
template<class T> void ObjectListModel<T>::refreshAll ()
{
	QModelIndex topLeft=createIndex (0, 0);
	QModelIndex bottomRight=createIndex (rowCount ()-1, columnCount () -1);

	emit dataChanged (topLeft, bottomRight);
}

/**
 * Maps a model index to the corresponding index in the list
 *
 * Since each row in the model corresponds to one entry in the list, this
 * depends only on the row of the index. The column of the index is ignored.
 */
template<class T> int ObjectListModel<T>::mapToSource (const QModelIndex &index)
{
	if (!index.isValid ())
		return -1;

	// That's easy, because the objectListModel's rows correspond to the source
	// model's entries 1:1.
	return index.row ();
}

/**
 * Maps an index in the list and a column to a model index
 */
template<class T> QModelIndex ObjectListModel<T>::mapFromSource (int sourceIndex, int column)
{
	if (sourceIndex<0)
		return QModelIndex ();

	// That's easy, because the objectListModel's rows correspond to the source
	// model's entries 1:1.
	return this->index (sourceIndex, column);
}

#endif
