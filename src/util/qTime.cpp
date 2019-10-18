#include "src/util/qTime.h"

#include <QTime>
#include <QDebug>

int toSeconds (const QTime &time)
{
    return QTime (0,0,0,0).secsTo (time);
}

QTime fromSeconds (int seconds)
{
    QTime result (0,0,0,0);
	result=result.addSecs (seconds);
	return result;
}
