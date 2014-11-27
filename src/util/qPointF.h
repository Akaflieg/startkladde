#ifndef QPOINTF_H_
#define QPOINTF_H_

class QPointF;

double min (const QPointF &point);
double max (const QPointF &point);
double lengthSquared (const QPointF &point);
QPointF transposed (const QPointF &point);

#endif
