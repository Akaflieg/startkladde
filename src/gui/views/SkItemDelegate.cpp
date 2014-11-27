/*
 * SkItemDelegate.cpp
 *
 *  Created on: 21.09.2010
 *      Author: martin
 */

#include "SkItemDelegate.h"

#include <QPainter>
#include <QModelIndex>

SkItemDelegate::SkItemDelegate (QObject *parent=NULL):
	QItemDelegate (parent),
	coloredSelectionEnabled (false)
{
}

SkItemDelegate::~SkItemDelegate ()
{
}

void SkItemDelegate::paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (coloredSelectionEnabled && (option.state & QStyle::State_Selected))
	{
		QStyleOptionViewItemV4 o (option);

		o.palette.setBrush (QPalette::HighlightedText, index.data (Qt::BackgroundRole).value<QBrush> ());
		o.palette.setColor (QPalette::Highlight, QColor (63, 63, 63));

		QItemDelegate::paint (painter, o, index);
	}
	else
	{
		QItemDelegate::paint (painter, option, index);
	}
}

// TODO make configurable
void SkItemDelegate::drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const
{
	Q_UNUSED (painter);
	Q_UNUSED (option);
	Q_UNUSED (rect);
}
