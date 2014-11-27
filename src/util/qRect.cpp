#include "qRect.h"

#include <cmath>

#include <QMargins>
#include <QPoint>
#include <QSize>

// Note that these methods do not use rect.right() and rect.bottom() because,
// other than for QRectF, "the values returned by [these] functions deviate from
// the true bottom-right corner of the rectangle" (see the Qt documentation for
// QRect). The recommended way to handle this problem is to use x()+width() for
// the right edge, and y()+height() for the bottom edge.

QRect northWestCorner (const QRect &rect, int width, int height)
{
	return QRect (rect.x (), rect.y (), width, height);
}

QRect northEastCorner (const QRect &rect, int width, int height)
{
	return QRect (rect.x ()+rect.width ()-width, rect.y (), width, height);
}

QRect southWestCorner (const QRect &rect, int width, int height)
{
	return QRect (rect.x (), rect.y ()+rect.height ()-height, width, height);
}

QRect southEastCorner (const QRect &rect, int width, int height)
{
	return QRect (rect.x ()+rect.width ()-width, rect.y ()+rect.height ()-height, width, height);
}



QRect northWestCorner (const QRect &rect, int size)
{
	return northWestCorner (rect, size, size);
}

QRect northEastCorner (const QRect &rect, int size)
{
	return northEastCorner (rect, size, size);
}

QRect southWestCorner (const QRect &rect, int size)
{
	return southWestCorner (rect, size, size);
}

QRect southEastCorner (const QRect &rect, int size)
{
	return southEastCorner (rect, size, size);
}



QRect northWestCorner (const QRect &rect, const QMargins &margins)
{
	return northWestCorner (rect, 2*margins.left  (), 2*margins.top    ());
}

QRect northEastCorner (const QRect &rect, const QMargins &margins)
{
	return northEastCorner (rect, 2*margins.right (), 2*margins.top    ());
}

QRect southWestCorner (const QRect &rect, const QMargins &margins)
{
	return southWestCorner (rect, 2*margins.left  (), 2*margins.bottom ());
}

QRect southEastCorner (const QRect &rect, const QMargins &margins)
{
	return southEastCorner (rect, 2*margins.right (), 2*margins.bottom ());
}


QRect centeredQRect (const QPoint &point, int size)
{
	return QRect (point.x ()-size/2, point.y ()-size/2, size, size);
}

QRect centeredQRect (const QPoint &point, const QSize &size)
{
	int x=point.x ();
	int y=point.y ();
	int w=size.width ();
	int h=size.height ();

	return QRect (x-w/2, y-h/2, w, h);
}


QRect enlarged (const QRect &rect, int margin)
{
	return QRect (
		rect.left ()-margin,
		rect.top  ()-margin,
		rect.width  ()+2*margin,
		rect.height ()+2*margin);
}
