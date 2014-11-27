#include "src/flarm/flarmMap/Kml.h"

#include <cmath>

// ***********
// ** Style **
// ***********

Kml::Style::Style ():
	lineWidth (1), noOutline (false), noFill (false)
{
}

/**
 * Returns a pen suitable for drawing the outline of the area defined by the
 * style
 *
 * If the area has no outline, a pen with the style Qt::NoPen is returned. If
 * the line color or width are still required, they can be read from the
 * lineColor and lineWidth properties, respectively.
 */
QPen Kml::Style::linePen () const
{
	if (noOutline)
		return QPen (Qt::NoPen);

	QPen pen (lineColor);
	pen.setWidth (round (lineWidth));
	return pen;
}

/**
 * Returns a brush suitable for filling the area defined by the style
 *
 * If the area is not filled, a brush with the style Qt::NoBrush is returned. If
 * the area color is still required, it can be read from the polyColor property.
 */
QBrush Kml::Style::polyBrush () const
{
	if (noFill)
		return QBrush (Qt::NoBrush);

	QBrush brush (polyColor);
	return brush;
}


// ***************
// ** Functions **
// ***************

QColor Kml::parseColor (const QString color)
{
	// It seems like fully opaque white may be stored as an empty string.
	if (color.isEmpty ())
		return Qt::white;

	if (color.length ()!=8)
		return QColor ();

	// aabbggrr
	int a=color.mid (0, 2).toInt (NULL, 16);
	int b=color.mid (2, 2).toInt (NULL, 16);
	int g=color.mid (4, 2).toInt (NULL, 16);
	int r=color.mid (6, 2).toInt (NULL, 16);

	return QColor (r, g, b, a);
}
