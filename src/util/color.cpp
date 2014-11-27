/*
 * color.cpp
 *
 *  Created on: 14.07.2010
 *      Author: Martin Herrmann
 */

#include "color.h"

#include <QColor>

#include "src/i18n/notr.h"

/**
 * Outputs a QColor to a stream in the form "r,g,b"
 *
 * @param s a stream
 * @param c a color
 * @return a reference to the stream
 */
std::ostream &operator<< (std::ostream &s, const QColor &c)
{
	if (c.isValid ())
		return s << c.red () << notr (",") <<  c.green () << notr (",") << c.blue ();
	else
		return s << notr ("-");
}

QColor interpol (float position, const QColor &color0, const QColor &color1)
{
	float p = position;
	float q = 1 - p;

	return QColor (
		color0.red   () * q + color1.red   () * p,
		color0.green () * q + color1.green () * p,
		color0.blue  () * q + color1.blue  () * p
		);
}

QColor interpol (float position, const QColor &color0, const QColor &color1, const QColor &color2)
{
	if (position <= 0.5)
		return interpol (2* position , color0, color1);
	else
		return interpol (2* (position -0.5), color1, color2);
}
