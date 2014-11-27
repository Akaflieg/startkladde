#ifndef IO_H_
#define IO_H_

#include <iostream>

#include <QString>

class QIODevice;
class QRegExp;
class QRect;

QString readLineUtf8 (QIODevice &device);
bool findInIoDevice (QIODevice &device, QRegExp &regexp);
QString findInIoDevice (QIODevice &device, const QRegExp &regexp, int group);

std::ostream &operator<< (std::ostream &ostream, const QRect &rect);

#endif
