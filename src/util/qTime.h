// Avoid conflicts with Qt headers
#ifndef UTIL_QTIME_H_
#define UTIL_QTIME_H_

class QTime;

int toSeconds (const QTime &time);
QTime fromSeconds (int seconds);

#endif
