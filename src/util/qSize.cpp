#include "qSize.h"

#include <cmath>

#include <QMargins>
#include <QRect>

QSize enlarged (const QSize &size, const QMargins &margins)
{
    return QSize (
        size.width  () + margins.left () + margins.right  (),
        size.height () + margins.top  () + margins.bottom ());
}

QSize min (const QSize &a, const QSize &b)
{
    return QSize (
        qMin (a.width  (), b.width  ()),
        qMin (a.height (), b.height ()));
}

QSize max (const QSize &a, const QSize &b)
{
    return QSize (
        qMax (a.width  (), b.width  ()),
        qMax (a.height (), b.height ()));
}

QPoint centeredIn (const QSize &rect, const QSize &container)
{
	return QPoint (
		(container.width  ()-rect.width  ())/2,
		(container.height ()-rect.height ())/2);
}

QPoint centeredIn (const QSize &rect, const QRect &container)
{
	return centeredIn (rect, container.size ()) + container.topLeft ();
}

double diameter (const QSize &size)
{
	double w=size.width ();
	double h=size.height ();

	return sqrt (w*w+h*h);
}
