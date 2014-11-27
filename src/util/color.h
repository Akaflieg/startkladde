#ifndef COLOR_H_
#define COLOR_H_

#include <iostream>

class QColor;

std::ostream &operator<< (std::ostream &s, const QColor &c);
QColor interpol (float position, const QColor &color0, const QColor &color1);
QColor interpol (float position, const QColor &color0, const QColor &color1, const QColor &color2);

#endif
