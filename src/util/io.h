#ifndef IO_H_
#define IO_H_

#include <iostream>

#include <QString>

class QIODevice;
class QRegExp;
class QRect;

QString readLineUtf8 (QIODevice &device);
std::optional<QRegularExpressionMatch> findInIoDevice (QIODevice &device, const QRegularExpression &regexp);
QString findInIoDeviceWithCapture (QIODevice &device, const QRegularExpression &regexp, int group);

std::ostream &operator<< (std::ostream &ostream, const QRect &rect);

#endif
