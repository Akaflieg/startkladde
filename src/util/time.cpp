#include "time.h"

#include <QTimeZone>
#include <cstdlib>

#include "src/i18n/notr.h"

/**
 * Returns a copy of a time with the seconds set to 0
 */
QTime nullSeconds (const QTime &time)
{
	if (!time.isValid ())
		return QTime ();

    return QTime (time.hour (), time.minute (), 0, 0);
}

/**
 * Returns a copy of a datetime with the seconds set to 0
 */
QDateTime nullSeconds (const QDateTime &dateTime)
{
    if (!dateTime.isValid()) {
		return QDateTime ();
    }

    return QDateTime(
        dateTime.date(),
        QTime(dateTime.time().hour(), dateTime.time().minute(), 0),
        dateTime.timeZone()
    );
}

/**
 * Returns the current time in the UTC time zone
 *
 * Note that QTime does not store time zone information.
 */
QTime currentTimeUtc ()
{
	return QDateTime::currentDateTime ().toUTC ().time ();
}

/**
 * Formats a duration
 *
 * @param seconds the duration in seconds
 * @param includeSeconds whether to include the seconds part
 * @return a string in the form "hhh:mm:ss" or "hhh:mm", depending on include
 *         Seconds
 */
QString formatDuration (int seconds, bool includeSeconds)
{
	bool negative=(seconds<0);
	seconds=std::abs (seconds);

	uint minutes=seconds/60;
	seconds=seconds%60;

	uint hours=minutes/60;
	minutes=minutes%60;

	QString sign=negative?notr ("-"):"";

	if (includeSeconds)
		return qnotr ("%1%2:%3:%4")
			.arg (sign)
			.arg (hours)
			.arg (minutes, 2, 10, QChar ('0'))
			.arg (seconds, 2, 10, QChar ('0'));
	else
		return qnotr ("%1%2:%3")
			.arg (sign)
			.arg (hours)
			.arg (minutes, 2, 10, QChar ('0'));
}


/**
 * Converts a UTC time of the current date to local time
 *
 * Note that QTime does not store time zone information.
 *
 * The results of this function may be unexpected on clock change day.
 *
 * @param time a UTC time
 * @return the corresponding local time
 */
QTime utcToLocal (const QTime &time)
{
	// TODO: currentDate probably returns local date
    QDateTime dateTime (QDate::currentDate(), time, QTimeZone::utc());
    return dateTime.toLocalTime().time();
}

// This function can be used, it's just not tested yet
///**
// * Converts a local time of the current date to UTC time
// *
// * Note that QTime does not store time zone information.
// *
// * The results of this function may be unexpected on clock change day.
// *
// * @param time a local time
// * @return the corresponding UTC time
// */
//QTime localToUtc (const QTime &time)
//{
//	// TODO: currentDate probably returns local date
//	QDateTime dateTime (QDate::currentDate (), time, Qt::LocalTime);
//	return dateTime.toUTC ().time ();
//}
