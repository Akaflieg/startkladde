#ifndef UTIL_QDATE_H_
#define UTIL_QDATE_H_

#include <QString>

#include "src/i18n/notr.h"

class QDate;

QDate validDate (const QDate &date, const QDate &invalidOption);
QString dateRangeToString (const QDate &first, const QDate &last, Qt::DateFormat format, const QString &separator);
QString dateRangeToString (const QDate &first, const QDate &last, const QString &format, const QString &separator);
QDate firstOfYear (int year);
QDate firstOfYear (const QDate &date);
std::ostream &operator<< (std::ostream &s, const QDate &date);

QString defaultNumericDateFormat ();
QString defaultPaddedNumericDateFormat ();

QString defaultNumericDateTimeFormat ();
QString defaultPaddedNumericDateTimeFormat ();

#endif
