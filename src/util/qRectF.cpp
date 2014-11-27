#include "qRectF.h"

#include <cmath>

#include <QMargins>

/**
 * Returns the rectangle with the given width and height whose upper left corner
 * coincides with the upper left corner of the given rectangle
 */
QRectF northWestCorner (const QRectF &rect, double width, double height)
{
	return QRectF (rect.left (), rect.top (), width, height);
}

/**
 * Returns the rectangle with the given width and height whose upper right
 * corner coincides with the upper right corner of the given rectangle
 */
QRectF northEastCorner (const QRectF &rect, double width, double height)
{
	return QRectF (rect.right ()-width, rect.top (), width, height);
}

/**
 * Returns the rectangle with the given width and height whose lower left corner
 * coincides with the lower left corner of the given rectangle
 */
QRectF southWestCorner (const QRectF &rect, double width, double height)
{
	return QRectF (rect.left (), rect.bottom ()-height, width, height);
}

/**
 * Returns the rectangle with the given width and height whose lower right
 * corner coincides with the lower right corner of the given rectangle
 */
QRectF southEastCorner (const QRectF &rect, double width, double height)
{
	return QRectF (rect.right ()-width, rect.bottom ()-height, width, height);
}


/**
 * Like northWestCorner (const QRectF &, double, double), but returns a square
 * with the given size as height and width
 */
QRectF northWestCorner (const QRectF &rect, double size)
{
	return northWestCorner (rect, size, size);
}

/**
 * Like northEastCorner (const QRectF &, double, double), but returns a square
 * with the given size as height and width
 */
QRectF northEastCorner (const QRectF &rect, double size)
{
	return northEastCorner (rect, size, size);
}

/**
 * Like southWestCorner (const QRectF &, double, double), but returns a square
 * with the given size as height and width
 */
QRectF southWestCorner (const QRectF &rect, double size)
{
	return southWestCorner (rect, size, size);
}

/**
 * Like southEastCorner (const QRectF &, double, double), but returns a square
 * with the given size as height and width
 */
QRectF southEastCorner (const QRectF &rect, double size)
{
	return southEastCorner (rect, size, size);
}



/**
 * Returns a rectangle centered at the given point, with the given size
 *
 * @see alignedQRectF
 */
QRectF centeredQRectF (const QPointF &point, const QSizeF &size)
{
	double x=point.x ();
	double y=point.y ();
	double w=size.width ();
	double h=size.height ();

	return QRectF (x-w/2, y-h/2, w, h);
}

/**
 * Returns a square centered at the given point, with side length size
 *
 * @see alignedQRectF
 */
QRectF centeredQRectF (const QPointF &point, double size)
{
	return centeredQRectF (point, QSizeF (size, size));
}

/**
 * Returns a rectangle with the given size, and aligned with the given point
 * according to the alignment flags
 *
 * The alignment parameter is a combination of a horizontal alignment flag
 * (Qt::AlignLeft, Qt::AlignHCenter or Qt::AlignRight) and a vertical alignment
 * flag (Qt::AlignTop, Qt::AlignVCenter or Qt::AlignBottom). If no flag is
 * specified for a direction (horizontal or vertical), the rectangle will be
 * centered in that direction. All other values are undefined.
 *
 * The rectangle will be aligned such that the given point is at the rectangle's
 * position specified by the alignment parameter. For example, if alignment is
 * Qt::AlignTop | Qt::AlignHCenter, the point will be at the center of the
 * rectangle's top side. Note that this means that the position of a rectangle
 * with Qt::AlignRight is farther to the left than with Qt::AlignLeft.
 *
 * Examples for valid alignment values are:
 *   - Qt::AlignTop | Qt::AlignLeft - the rectangle's top left corner is a the
 *     point
 *   - Qt::AlignLeft | Qt::AlignVCenter - the center of the rectangle's left
 *     side is at the point
 *   - Qt::AlignLeft - same as Qt::AlignLeft | Qt::AlignVCenter
 *   - Qt::AlignVCenter | Qt::AlignHCenter - the center of the rectangle is at
 *     the point
 *   - 0 - same as Qt::AlignVCenter | Qt::AlignHCenter
 *
 * @see centeredQRectF
 */
QRectF alignedQRectF (const QPointF &point, Qt::Alignment alignment, const QSizeF &size)
{
	double x=point.x ();
	double y=point.y ();
	double w=size.width ();
	double h=size.height ();

	double rx;
	if      (alignment & Qt::AlignLeft ) rx=x;     // Left aligned
	else if (alignment & Qt::AlignRight) rx=x-w;   // Right aligned
	else                                 rx=x-w/2; // Centered horizontally

	double ry;
	if      (alignment & Qt::AlignTop   ) ry=y;     // Top aligned
	else if (alignment & Qt::AlignBottom) ry=y-h;   // Bottom aligned
	else                                  ry=y-h/2; // Centered vertically

	return QRectF (rx, ry, w, h);
}

/**
 * Same as alignedQRectF (const QPointF &, Qt::Alignment, const QSizeF &), but
 * returns a square with the given size as width and height
 */
QRectF alignedQRectF (const QPointF &point, Qt::Alignment alignment, double size)
{
	return alignedQRectF (point, alignment, QSizeF (size, size));
}


/**
 * Returns a copy of the rectangle, with the position (left and top) and the
 * size (width and height) rounded
 */
QRectF round (const QRectF &rect)
{
	double left  =round (rect.left   ());
	double top   =round (rect.top    ());
	double width =round (rect.width  ());
	double height=round (rect.height ());

	return QRectF (left, top, width, height);
}


/**
 * Returns a copy of the rectangle, with the margin added on all sides
 */
QRectF enlarged (const QRectF &rect, double margin)
{
	return QRectF (
		rect.left ()-margin,
		rect.top  ()-margin,
		rect.width  ()+2*margin,
		rect.height ()+2*margin);
}

/**
 * Calculates the minimum distance from the specified point to any point in the
 * specified rectangle
 */
double minimumDistance (const QRectF &rect, const QPointF &point)
{
	// Get the relevant coordinates of the point and the rectangle. Note that
	// the coordinate system has y down, so bottom>=top.
	double x=point.x ();
	double y=point.y ();
	double left=rect.left ();
	double right=rect.right ();
	double top=rect.top ();
	double bottom=rect.bottom ();

	double dx, dy;

	if      (x<left ) dx=left-x;  // The point is left of the rectangle
	else if (x>right) dx=x-right; // The point is right of the rectangle
	else              dx=0;       // The point is horizontally within the rectangle

	if      (y<top)    dy=top-y;    // The point is above the rectangle
	else if (y>bottom) dy=y-bottom; // The point is below the rectangle
	else               dy=0;        // The point is vertically within the rectangle

	return sqrt (dx*dx+dy*dy);
}

/**
 * Calculates the minimum distance from the specified point to any point in the
 * specified rectangle
 */
double maximumDistance (const QRectF &rect, const QPointF &point)
{
	// The distance is always the distance to a corner

	// Get the relevant coordinates of the point and the rectangle. Note that
	// the coordinate system has y down, so bottom>=top.
	double x=point.x ();
	double y=point.y ();
	double left=rect.left ();
	double right=rect.right ();
	double top=rect.top ();
	double bottom=rect.bottom ();
	QPointF center=rect.center ();

	double dx, dy;

	if (x>=center.x ()) dx=x-left;
	else                dx=right-x;

	if (y>=center.y ()) dy=y-top;
	else                dy=bottom-y;

	return sqrt (dx*dx+dy*dy);
}

/**
 * Returns the line from the top left to the top right corner
 */
QLineF topLine (const QRectF &rect)
{
	return QLineF (rect.topLeft (), rect.topRight ());
}

/**
 * Returns the line from the top right to the bottom right corner
 */
QLineF rightLine (const QRectF &rect)
{
	return QLineF (rect.topRight (), rect.bottomRight ());
}

/**
 * Returns the line from the bottom right to the bottom left corner
 */
QLineF bottomLine (const QRectF &rect)
{
	return QLineF (rect.bottomRight (), rect.bottomLeft ());
}

/**
 * Returns the line from the bottom left to the top left corner
 */
QLineF leftLine (const QRectF &rect)
{
	return QLineF (rect.bottomLeft (), rect.topLeft ());
}
