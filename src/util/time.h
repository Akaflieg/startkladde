/*
 * time.h
 *
 *  Created on: 22.03.2010
 *      Author: deffi
 */

#ifndef TIME_H_
#define TIME_H_

#include <QTime>

class Longitude;

QTime nullSeconds (const QTime &time);
QDateTime nullSeconds (const QDateTime &dateTime);

QTime currentTimeUtc ();
QTime utcToLocal (const QTime &time);
//QTime localToUtc (const QTime &time); // Not tested


QString formatDuration (int seconds, bool includeSeconds=true);



#endif
