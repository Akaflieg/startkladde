#ifndef SKITEMDELEGATE_H_
#define SKITEMDELEGATE_H_

#include <QItemDelegate>

#include "src/accessor.h"

class QPainter;
class QStyleOptionViewItem;
class QModelIndex;

/**
 * A QItemDelegate subclass which adds the following functionality:
 *   - selected cells are displayed with a dark gray background and the actual
 *     background color as foreground (if enabled via coloredSelectionEnabled)
 */
class SkItemDelegate: public QItemDelegate
{
	public:
		SkItemDelegate (QObject *parent);
		virtual ~SkItemDelegate ();

		virtual void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

		value_accessor (bool, ColoredSelectionEnabled, coloredSelectionEnabled);

		void drawFocus (QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const;

	private:
		bool coloredSelectionEnabled;
};

#endif
