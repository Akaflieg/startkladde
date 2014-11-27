#include "src/gui/widgets/NotificationsLayout.h"

#include <iostream>
#include <cmath>
#include <iomanip>

#include "src/util/math.h"
#include "src/algorithms/RectangleLayout.h"

// Improvements:
//   * we can perform the layout automatically, by adding a geometryChanged
//     event to NotificationWidget

// TODO remove widgets automatically when they are deleted (and document)

/**
 * Creates a new NotificationsLayout without any notifications to lay out
 */
NotificationsLayout::NotificationsLayout ()
{
}

NotificationsLayout::~NotificationsLayout ()
{
}

/**
 * Adds a notification widget to the layout
 *
 * All widgets added to the same layout should have the same parent, or the
 * results may be unexpected.
 *
 * This class does not take ownership of the widget.
 *
 * The initial arrow position of the widget is undefined; you must call
 * setArrowPosition or setWidgetInvisible.
 *
 * @see remove
 * @see setArrowPosition
 * @see setWidgetInvisible
 */
void NotificationsLayout::add (NotificationWidget *widget)
{
	Node node;

	node.widget=widget;
	node.visible=false;

	_nodes.insert (widget, node);
}

/**
 * Removes a notification widget from the layout
 *
 * The widget will not be considered for layout calculations and will not be
 * moved. It is not deleted; this is its parent's task.
 *
 * @see add
 */
void NotificationsLayout::remove (const NotificationWidget *widget)
{
	_nodes.remove (widget);
}

/**
 * Sets the arrow position for the given widget
 *
 * The widget must have been added using the add method. If the widget was
 * invisible before, it will be shown on the next call to doLayout().
 *
 * @see add
 */
void NotificationsLayout::setArrowPosition (const NotificationWidget *widget, const QPoint &position)
{
	Node &node=_nodes[widget];

	node.arrowPosition=position;
	node.visible=true;
}

/**
 * Sets the specified widget to be invisible
 *
 * The widget must have been added using the add method. If the widget was
 * visible before, it will be hidden on the next call to doLayout().
 */
void NotificationsLayout::setWidgetInvisible (const NotificationWidget *widget)
{
	Node &node=_nodes[widget];

	node.visible=false;
}

/**
 * Performs the actual layout
 *
 * This method also shows or hides the widgets, depending on whether
 * setArrowPosition or setWidgetInvisible was called on the widget last.
 *
 *
 * This is an expensive method. Do not call it in vain.
 */
void NotificationsLayout::doLayout ()
{
	QList<Node> visibleNodes;

	// Iterate over all nodes. Make a list of visible nodes and hide all
	// invisible nodes.
	QList<Node> nodes=_nodes.values ();
	for (int i=0, n=nodes.size (); i<n; ++i)
	{
		Node &node=nodes[i];

		if (node.visible)
			visibleNodes.append (node);
		else
			node.widget->hide ();
	}

	// Add the visible nodes to a rectangle layout
	RectangleLayout layout;
	layout.setSpacing (1);
	for (int i=0, n=visibleNodes.size (); i<n; ++i)
	{
		const Node &node=visibleNodes[i];

		QPoint arrowPosition=node.arrowPosition;
		NotificationWidget *widget=node.widget;

		int targetY=widget->defaultBubblePosition (arrowPosition).y ();
		int h=widget->bubbleGeometryParent ().height ();

		layout.addItem (targetY, h);
	}

	// Perform the layout
	layout.doLayout (50);

	// Apply the layout
	QList<RectangleLayout::Item> layoutItems=layout.items ();
	for (int i=0, n=layoutItems.size (); i<n; ++i)
	{
		RectangleLayout::Item &item=layoutItems[i];

		Node &node=visibleNodes[item.originalIndex];

		QPoint arrowPosition=node.arrowPosition;
		QPoint bubblePosition=node.widget->defaultBubblePosition (arrowPosition);

		bubblePosition.setY (item.y);

		node.widget->moveTo (arrowPosition, bubblePosition);
		node.widget->show ();
	}
}
