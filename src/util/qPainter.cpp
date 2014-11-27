#include "src/util/qPainter.h"

#include <iostream>
#include <cmath>

#include <QDebug>
#include <QPainter>
#include <QPoint>
#include <QPointF>
#include <QString>

#include "src/util/qRect.h"
#include "src/util/qRectF.h"


// ************
// ** Shapes **
// ************

/**
 * Draws a cross, consisting of a vertical and a horizontal line
 *
 * The center of the cross is at the specified position. The size specifies the
 * distance from the center of the cross to the end of the lines.
 */
void drawOrthogonalCross (QPainter &painter, const QPoint &position, int size)
{
	QPoint dx (size, 0);
	QPoint dy (0, size);
	painter.drawLine (position-dx, position+dx);
	painter.drawLine (position-dy, position+dy);
}

/**
 * Draws a cross, consisting of a vertical and a horizontal line
 *
 * The center of the cross is at the specified position. The size specifies the
 * distance from the center of the cross to the end of the lines.
 */
void drawOrthogonalCross (QPainter &painter, const QPointF &position, double size)
{
	QPointF dx (size, 0);
	QPointF dy (0, size);
	painter.drawLine (position-dx, position+dx);
	painter.drawLine (position-dy, position+dy);
}

/**
 * Draws a cross, consisting of to perpendicular diagonal lines
 *
 * The center of the cross is at the specified position. The size specifies the
 * distance from the center of the cross to the end of the lines.
 */
void drawDiagonalCross (QPainter &painter, const QPoint &position, int size)
{
	size=size/M_SQRT2;
	QPoint dx (size, size);
	QPoint dy (size, -size);
	painter.drawLine (position-dx, position+dx);
	painter.drawLine (position-dy, position+dy);
}

/**
 * Draws a cross, consisting of to perpendicular diagonal lines
 *
 * The center of the cross is at the specified position. The size specifies the
 * distance from the center of the cross to the end of the lines.
 */
void drawDiagonalCross (QPainter &painter, const QPointF &position, double size)
{
	size=size/M_SQRT2;
	QPointF dx (size, size);
	QPointF dy (size, -size);
	painter.drawLine (position-dx, position+dx);
	painter.drawLine (position-dy, position+dy);
}

/**
 * Draws a circle
 */
void drawCircle (QPainter &painter, const QPoint &center, int radius)
{
	painter.drawArc (centeredQRect (center, radius), 0, 16*360);
}

/**
 * Draws a circle
 */
void drawCircle (QPainter &painter, const QPointF &center, double radius)
{
	painter.drawArc (centeredQRectF (center, 2*radius), 0, 16*360);
}


// **********
// ** Text **
// **********

/**
 * Determines the size of a text when painted with the current painter
 *
 * The text may contain newlines.
 */
QSize textSize (const QPainter &painter, const QString &text)
{
	return painter.fontMetrics ().size (0, text);
}

/**
 * Same as drawCenteredText (QPainter &, const QPointF &, const QString &,
 * double), but with integer arguments
 */
void drawCenteredText (QPainter &painter, const QPoint &position, const QString &text, int margin)
{
	drawCenteredText (painter, QPointF (position), text, margin);
}

/**
 * Draws a string at the specified position
 *
 * The text will be drawn with the center at the specified position. If the text
 * contains newlines, the individual lines will be centered.
 *
 * The text rectangle plus the specified margin is filled with the background
 * brush.
 */
void drawCenteredText (QPainter &painter, const QPointF &position, const QString &text, double margin)
{
	QSize size=textSize (painter, text);
	QRectF rect=centeredQRectF (position, QSizeF (size));
	// It seems like drawText, while it accepts a QRectF, is not able to draw to
	// subpixel positions. This causes jumping text due to rounding issues. In
	// order to at least keep text and background rectangle coincident, we round
	// the position manually.
	rect=round (rect);
	painter.fillRect (enlarged (rect, margin), painter.brush ());
	painter.drawText (rect, Qt::AlignHCenter, text);
}

/**
 * Same as drawText (QPainter &, const QPointF &, Qt::Alignment, const
 * QString &, double), but with integer arguments
 */
void drawText (QPainter &painter, const QPoint &position, Qt::Alignment alignment, const QString &text, int margin)
{
	return drawText (painter, QPointF (position), alignment, text, margin);
}

/**
 * Draws a string of text at the specified position
 *
 * The text will be drawn aligned with the specified position according to the
 * alignment parameter. For alignment, the same rules as for alignedQRectF
 * apply.
 *
 * The text rectangle plus the specified margin is filled with the background
 * brush.
 */
void drawText (QPainter &painter, const QPointF &position, Qt::Alignment alignment, const QString &text, double margin)
{
	QSize size=textSize (painter, text);
	QRectF rect=alignedQRectF (position, alignment, QSizeF (size));
	// It seems like drawText, while it accepts a QRectF, is not able to draw to
	// subpixel positions. This causes jumping text due to rounding issues. In
	// order to at least keep text and background rectangle coincident, we round
	// the position manually.
	rect=round (rect);
	painter.fillRect (enlarged (rect, margin), painter.brush ());
	painter.drawText (rect, Qt::AlignHCenter, text);
}
