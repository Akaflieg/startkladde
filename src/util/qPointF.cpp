#include <src/util/qPointF.h>

#include <QPointF>

/**
 * Returns the smaller of a point's x and y values
 */
double min (const QPointF &point)
{
	return qMin (point.x (), point.y ());
}

/**
 * Returns the bigger of a point's x and y values
 */
double max (const QPointF &point)
{
	return qMax (point.x (), point.y ());
}

/**
 * Determines the square of the euclidian length of a point
 *
 * The square of the length can be calculated more quickly than the length and
 * it may be sufficient for calculations and comparisons.
 */
double lengthSquared (const QPointF &point)
{
	return point.x ()*point.x () + point.y ()*point.y ();
}

/**
 * Returns a QPointF with swapped x and y values
 */
QPointF transposed (const QPointF &point)
{
	return QPointF (point.y (), point.x ());
}
