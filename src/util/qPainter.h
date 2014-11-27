#ifndef UTIL_QPAINTER_H_
#define UTIL_QPAINTER_H_

#include <QSize>

class QPainter;
class QPoint;
class QPointF;
class QString;

// Shapes
void drawOrthogonalCross (QPainter &painter, const QPoint  &position, int    size);
void drawOrthogonalCross (QPainter &painter, const QPointF &position, double size);
void drawDiagonalCross (QPainter &painter, const QPoint  &position, int    size);
void drawDiagonalCross (QPainter &painter, const QPointF &position, double size);
void drawCircle (QPainter &painter, const QPoint  &center, int    radius);
void drawCircle (QPainter &painter, const QPointF &center, double radius);

// Text
QSize textSize (const QPainter &painter, const QString &text);
void drawCenteredText (QPainter &painter, const QPoint  &position, const QString &text, int    margin=0);
void drawCenteredText (QPainter &painter, const QPointF &position, const QString &text, double margin=0);
void drawText (QPainter &painter, const QPoint  &position, Qt::Alignment alignment, const QString &text, int   margin=0);
void drawText (QPainter &painter, const QPointF &position, Qt::Alignment alignment, const QString &text, double margin=0);

#endif
