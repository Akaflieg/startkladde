#ifndef _QSIZE_H
#define _QSIZE_H

#include <QSize>
#include <QPoint>

class QMargins;
class QRect;

QSize enlarged (const QSize &size, const QMargins &margins);
QSize max (const QSize &a, const QSize &b);
QPoint centeredIn (const QSize &rect, const QSize &container);
QPoint centeredIn (const QSize &rect, const QRect &container);
double diameter (const QSize &size);

#endif
