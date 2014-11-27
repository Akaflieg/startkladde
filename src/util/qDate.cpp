#include "qDate.h"

#include <QApplication>
#include <QDate>

#include "src/util/qString.h"

/**
 * Returns date if it is valid, or invalidOption if date is not valid
 */
QDate validDate (const QDate &date, const QDate &invalidOption)
{
	if (date.isValid ())
		return date;
	else
		return invalidOption;
}

QString dateRangeToString (const QDate &first, const QDate &last, Qt::DateFormat format, const QString &separator)
{
	if (!first.isValid () || !last.isValid ())
		return qApp->translate ("QDate", "invalid");
	else if (first==last)
		return first.toString (format);
	else
		return first.toString (format)+separator+last.toString (format);
}

QString dateRangeToString (const QDate &first, const QDate &last, const QString &format, const QString &separator)
{
	if (!first.isValid () || !last.isValid ())
		return qApp->translate ("QDate", "invalid");
	else if (first==last)
		return first.toString (format);
	else
		return first.toString (format)+separator+last.toString (format);
}

QDate firstOfYear (int year)
{
	return QDate (year, 1, 1);
}

QDate firstOfYear (const QDate &date)
{
	return firstOfYear (date.year ());
}

std::ostream &operator<< (std::ostream &s, const QDate &date)
{
	return s << date.toString (Qt::ISODate);
}

// We use our own date/time format strings instead of Qt::DefaultLocaleLongDate
// et at. because:
//   * QTime::toString (Qt::DefaultLocaleLongDate) includes the time zone,
//     which will be reported as the local time zone even for UTC time, since
//     QTime doesn't know about time zones.
//   * for some locales, Qt::DefaultLocaleShortDate uses a (short) month name,
//     while for others, it uses the number.
// We implement this as a function rather than a constant string because
//   * a const QString with QT_TR_NOOP needs to be wrapped in tr() everywhere
//     it is used, and we don't have a way to check this (cf. script/
//     find_missing_tr)
//   * a #define will not be picked up by lupdate
//   * we may want to be able to configure 12/24 hour time format

QString defaultNumericDateFormat ()
{
	return qApp->translate ("QDate", "M/d/yyyy");
}

QString defaultPaddedNumericDateFormat ()
{
	return qApp->translate ("QDate", "MM/dd/yyyy");
}

QString defaultNumericDateTimeFormat ()
{
	return qApp->translate ("QDate", "M/d/yyyy h:mm:ss"); // Minutes and seconds are always padded
}

QString defaultPaddedNumericDateTimeFormat ()
{
	return qApp->translate ("QDate", "MM/dd/yyyy hh:mm:ss");
}


