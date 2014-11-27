#ifndef _QMARGINS_H
#define _QMARGINS_H

#include <QSize>
#include <QMargins>

class QWidget;

QSize minimumSize (const QMargins &margins);
QMargins marginsFromStyle (const QWidget *widget);

#endif
