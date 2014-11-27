#include "ReadOnlyItemDelegate.h"

ReadOnlyItemDelegate::ReadOnlyItemDelegate (QObject *parent):
	QItemDelegate (parent)
{
}

ReadOnlyItemDelegate::~ReadOnlyItemDelegate ()
{
}

QWidget *ReadOnlyItemDelegate::createEditor (QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	(void)parent;
	(void)option;
	(void)index;
	return NULL;
}
