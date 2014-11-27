/*
 * ReadOnlyItemDelegate.h
 *
 *  Created on: 16.03.2010
 *      Author: Martin Herrmann
 */

#ifndef READONLYITEMDELEGATE_H_
#define READONLYITEMDELEGATE_H_

#include <QItemDelegate>

/**
 * A QItemDelegate subclass that returns NULL from createEditor
 *
 * This is required for bool columns (containing a checkbox) in an item view.
 * The checkbox (checkedState) is independent from the value of the item, so
 * usually, the item can still be edited independent from the checkbox. Setting
 * the item flag to ~ItemIsEditable also disables the checkbox, even if
 * ItemIsUserCheckable is set. Using this delegate prevents editing of the
 * value while still allowing access to the checkbox.
 */
class ReadOnlyItemDelegate: public QItemDelegate
{
	public:
		ReadOnlyItemDelegate (QObject *parent=NULL);
		virtual ~ReadOnlyItemDelegate ();

		virtual QWidget *createEditor (QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif
