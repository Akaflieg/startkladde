#ifndef NOTIFICATIONSLAYOUT_H_
#define NOTIFICATIONSLAYOUT_H_

#include <QHash>
#include <QPoint>

#include "src/gui/widgets/NotificationWidget.h"

/**
 * Lays out multiple notification widgets so they don't overlap
 *
 * To use this class, add the widgets you want laid out using add(). For each
 * widget, either set its arrow position using setArrowPosition() if you want it
 * visible, or hide it using setWidgetInvisible() if not.
 *
 * Call doLayout to actually perform the layout. This will only move the
 * widgets, as their size is fixed. Since this is not a QLayout, you have to do
 * this manually.

 * The notification widgets will be placed in such a way that the arrow
 * positions are at the specified position and the bubble positions are as close
 * as possible to the recommended position for the respective arrow positions,
 * but making sure that none of the visible bubbles overlap. Note that the
 * widgets themselves may (and typically will) still overlap to accommodate the
 * arrows.
 *
 * Note that this class is not a layout manager in the sense of the Qt layout
 * management API (nor does it inherit from QLayout). A parent with notification
 * widgets will typically have another layout for its "regular" child widgets.
 *
 * This currently assumes that the x coordinates of the arrow positions are
 * equal. This is not a requirement, but the layout results may be unexpected as
 * only the vertical direction is considered for layout.
 *
 * This class uses RectangleLayout to calculate the layout.
 *
 * @see NotificationWidget
 * @see RectangleLayout
 */
class NotificationsLayout
{
	public:
		NotificationsLayout ();
		virtual ~NotificationsLayout ();

		void add (NotificationWidget *widget);
		void remove (const NotificationWidget *widget);

		void setArrowPosition (const NotificationWidget *widget, const QPoint &position);
		void setWidgetInvisible (const NotificationWidget *widget);

		void doLayout ();


	private:
		class Node
		{
			public:
				NotificationWidget *widget;
				bool visible;
				QPoint arrowPosition;
		};

		QHash<const NotificationWidget *, Node> _nodes;
};

#endif
