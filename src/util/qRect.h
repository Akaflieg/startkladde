#ifndef QRECTF_H_
#define QRECTF_H_

#include <QRect>

class QPoint;
class QSize;
class QMargins;

QRect northWestCorner (const QRect &rect, int width, int height);
QRect northEastCorner (const QRect &rect, int width, int height);
QRect southWestCorner (const QRect &rect, int width, int height);
QRect southEastCorner (const QRect &rect, int width, int height);

QRect northWestCorner (const QRect &rect, int size);
QRect northEastCorner (const QRect &rect, int size);
QRect southWestCorner (const QRect &rect, int size);
QRect southEastCorner (const QRect &rect, int size);

QRect northWestCorner (const QRect &rect, const QMargins &margins);
QRect northEastCorner (const QRect &rect, const QMargins &margins);
QRect southWestCorner (const QRect &rect, const QMargins &margins);
QRect southEastCorner (const QRect &rect, const QMargins &margins);

QRect centeredQRect (const QPoint &point, int size);
QRect centeredQRect (const QPoint &point, const QSize &size);

QRect enlarged (const QRect &rect, int margin);

#endif
