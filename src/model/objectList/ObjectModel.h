/*
 * ObjectModel.h
 *
 *  Created on: Aug 19, 2009
 *      Author: Martin Herrmann
 */

#ifndef OBJECTMODEL_H_
#define OBJECTMODEL_H_

#include <QVariant>
#include <QStringList>

/**
 * A view on the data of a class - providing data and header data for multiple
 * roles. This is suitable for use with QItemViews as it provides data for
 * different roles.
 *
 * This is implemented as non-static method of a model class rather than with
 * static methods for the following reasons:
 *   - it's possible to use an abstract base class, providing more type safety
 *     and easier diagnosticts
 *   - methods can be inherited, so common or default functionality is possible
 *   - it's possible to set parameters (properties of this class), for example
 *     the unit to show a physical quantity in
 *   - the runtime overhead over a static method solution is small
 *   - it is possible to select from different models for the same view at
 *     runtime
 *
 * Implementations can be passed as reference or as template parameter. Passing
 * by reference is usually better.
 *
 * Notes for implementations of this class:
 *   - all implementations must implement the columnCount method
 *   - for a full-featured implementation, implementations will typically
 *     override the data and headerData methods. An example is FlarmRecordModel,
 *     which also uses TextAlignmentRole.
 *   - if DisplayRole is the only role used, an implementation can override
 *     the displayData and displayHeaderData methods and use the default
 *     implementations of data and headerData which call displayData and
 *     displayHeaderDate, respectively, for the DisplayRole. An example is
 *     PersonModel.
 */
template<class T> class ObjectModel
{
	public:
		virtual ~ObjectModel ();

		/**
		 * Determines the number of columns in this model.
		 * @return the number of columns in this model
		 */
		virtual int columnCount () const=0;

		virtual QVariant headerData (int column, int role=Qt::DisplayRole) const;
		virtual QVariant data (const T &object, int column, int role=Qt::DisplayRole) const;

		virtual QStringList displayHeaderStrings () const;
		virtual QStringList displayDataStrings (const T &object) const;

	protected:
		virtual QVariant displayHeaderData (int column) const;
		virtual QVariant displayData (const T &object, int column) const;
};

template<class T> ObjectModel<T>::~ObjectModel ()
{
}

/**
 * Returns the data for the header of the model (e. g. column names). The
 * default implementations returns the result of displayHeaderData for the
 * Qt::DisplayRole and an invalid QVariant for all other roles.
 *
 * @param column the column for which to return the header data; must be >=0 and
 *               <columnCount
 * @param role the role for which to return the header data
 * @return the header data for the column and role specified
 * @see displayHeaderData
 */
template<class T> QVariant ObjectModel<T>::headerData (int column, int role) const
{
	if (role==Qt::DisplayRole) return displayHeaderData (column);
	return QVariant ();
}

/**
 * Returns the data for one row of the model (corresponding to one object). The
 * default implementation returns the result of displayData for the
 * Qt::DisplayRole and an invalid QVariant for all other roles.
 *
 * @param object the object to get the data from
 * @param column the columns for which to return the data; must be >=0 and
 *               <columnCount
 * @param role the role for which to return the data
 * @return the data for the object, column and role specified
 * @see displayData
 */
template<class T> QVariant ObjectModel<T>::data (const T &object, int column, int role) const
{
	if (role==Qt::DisplayRole) return displayData (object, column);
	return QVariant ();
}

/**
 * Returns the header data for the display role, that is, the column name, for
 * use with the default headerData implementation.
 *
 * The default implementation returns the column number, starting at 1.
 *
 * @param column the number of the column, starting a 0, and less then
 *        columnCount ()
 * @return the column name
 */
template<class T> QVariant ObjectModel<T>::displayHeaderData (int column) const
{
	return column+1;
}

/**
 * Returns the data for the display role for a given object.
 *
 * The default implementation returns an invalid QVariant object and is given
 * so this method does not have to be implemented if the generic data method
 * is implemented.
 *
 * @param object the object
 * @param column the column number, starting at 0, and less than columnCount ()
 * @return the data for the display role for the given column of the given
 *         object
 */
// TODO make it pure virtual, it produced hard to debug errors if the signature
// does not match
template<class T> QVariant ObjectModel<T>::displayData (const T &object, int column) const
{
	(void)object;
	(void)column;
	return QVariant ();
}

/**
 * Returns the header display data as a string list
 *
 * @return a list of string representations of the header data for the
 *         DisplayRole. The size of the list is equal to columnCount ().
 */
template<class T> QStringList ObjectModel<T>::displayHeaderStrings () const
{
	QStringList result;

	for (int i=0; i<columnCount (); ++i)
		result.append (headerData (i).toString ());

	return result;
}


/**
 * Returns the display data for an object as a string list
 *
 * @param object an object
 * @return a list of string representations of the data for the DisplayRole of
 *         object. The size of the list is equal to columnCount ().
 */
template<class T> QStringList ObjectModel<T>::displayDataStrings (const T &object) const
{
	QStringList result;

	for (int i=0; i<columnCount (); ++i)
		result.append (data (object, i).toString ());

	return result;
}

#endif
