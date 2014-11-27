#ifndef UTIL_QRECTF_H_
#define UTIL_QRECTF_H_

#include <QLineF>
#include <QRect>

class QMargins;

QRectF northWestCorner (const QRectF &rect, double width, double height);
QRectF northEastCorner (const QRectF &rect, double width, double height);
QRectF southWestCorner (const QRectF &rect, double width, double height);
QRectF southEastCorner (const QRectF &rect, double width, double height);

QRectF northWestCorner (const QRectF &rect, double size);
QRectF northEastCorner (const QRectF &rect, double size);
QRectF southWestCorner (const QRectF &rect, double size);
QRectF southEastCorner (const QRectF &rect, double size);

QRectF centeredQRectF (const QPointF &point, double size);
QRectF centeredQRectF (const QPointF &point, const QSizeF &size);

QRectF alignedQRectF (const QPointF &point, Qt::Alignment alignment, double size);
QRectF alignedQRectF (const QPointF &point, Qt::Alignment alignment, const QSizeF &size);

QRectF round (const QRectF &rect);

QRectF enlarged (const QRectF &rect, double margin);

double minimumDistance (const QRectF &rect, const QPointF &point);
double maximumDistance (const QRectF &rect, const QPointF &point);

QLineF topLine    (const QRectF &rect);
QLineF bottomLine (const QRectF &rect);
QLineF leftLine   (const QRectF &rect);
QLineF rightLine  (const QRectF &rect);

#endif
