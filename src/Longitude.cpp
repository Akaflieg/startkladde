/*
 * Longitude.cpp
 *
 *  Created on: 13.07.2010
 *      Author: Martin Herrmann
 */

#include "Longitude.h"

#include <cstdlib>
#include <cmath>

#include <QRegularExpression>
#include <QDebug>

#include "src/i18n/notr.h"
#include "src/util/qString.h"


// ******************
// ** Construction **
// ******************

/**
 * Creates an invalid longitude
 */
Longitude::Longitude ():
	value (0), valid (false)
{
}

/**
 * Creates a longitude with a given value
 *
 * @param value the longitude value in degrees
 */
Longitude::Longitude (double value):
	value (value), valid (true)
{
}

/**
 * Creates a longitude with a value given as degrees, minutes and seconds and
 * a sign
 */
Longitude::Longitude (unsigned int degrees, unsigned int minutes, unsigned int seconds, bool positive):
	valid (true)
{
	value = degrees + minutes/(double)60 + seconds/(double)3600;
	if (!positive) value=-value;
}

Longitude::~Longitude ()
{
}


// *********************
// ** Data processing **
// *********************

/**
 * Normalizes the longitude
 *
 * A normalized longitude is larger than -180° and less than or equal to +180°.
 *
 * @return a new Longitude representing the same longitude, but normalized
 */
Longitude Longitude::normalized () const
{
	double newValue=fmod (value, 360);

	if (newValue>180)
		newValue-=360;
	else if (newValue<=-180)
		newValue+=360;

	return Longitude (newValue);
}

/**
 * Determines the difference in solar time between two longitudes at the same
 * clock time (expressed in the same time zone), that is, at the same point in
 * time
 *
 * For a longitude east of the other longitude, the solar time is later, thus,
 * the returned value is positive.
 *
 * @param other another longitude
 * @return the difference in solar time, in seconds
 * @see clockTimeOffsetTo
 */
double Longitude::solarTimeOffsetTo (const Longitude &other)
{
	double dLon=normalized ().getValue ()-other.normalized ().getValue ();

	// 360° -- 1 day (86400 s)
	return dLon/360*86400;
}

/**
 * Determines the difference in clock time (expressed in the same time zone)
 * between two longitudes at the same solar time
 *
 * For a longitude east of the other longitude, the same solar time corresponds
 * to an earlier clock time, so the returned value is negative.
 *
 * @param other another longitude
 * @return the difference in clock time, in seconds
 * @see solarTimeOffsetTo
 */
double Longitude::clockTimeOffsetTo (const Longitude &other)
{
	return -solarTimeOffsetTo (other);
}


// ****************
// ** Conversion **
// ****************

/**
 * Converts the longitude to a degrees/minutes/seconds representation with
 * minutes and seconds in the 0..59 range. The seconds are rounded
 * mathematically, rounding towards higher numbers for 0.5 seconds.
 *
 * No normalization is performed. Note that it is possible to get -180°0'0"
 * even with a normalized value.
 *
 * @param degrees the degrees are written here
 * @param minutes the minutes are written here
 * @param seconds the seconds are written here
 * @param positive set to true if the value is greater than or equal to 0,
 *                 false else
 * @param remainder the amount the returned value is too small
 */
void Longitude::toDms (uint &degrees, uint &minutes, uint &seconds, bool &positive, double &remainder) const
{
	// Calculate the value in seconds
	double valueSeconds=value*3600;

	// Round the seconds mathematically, rounding towards higher numbers for
	// 0.5 seconds
	int rounded=floor (valueSeconds+0.5);

	// Determine the rounding remainder
	remainder=(valueSeconds-rounded)/3600;

	// Note and remove the sign of the rounded value
	positive=(rounded>=0);
	rounded=abs (rounded);

	// Determine the seconds
	seconds=rounded%60;

	// Determine the minutes
	rounded=rounded/60;
	minutes=rounded%60;

	// Determine the degrees
	rounded=rounded/60;
	degrees=rounded;
}

void Longitude::toDms (uint &degrees, uint &minutes, uint &seconds, bool &positive) const
{
	double remainder;
	toDms (degrees, minutes, seconds, positive, remainder);
}

/**
 * Creates a string suitable for display to the user
 *
 * @param positiveString the designator to add for eastern longitudes
 * @param negativeString the designator to add for western longitudes
 * @return a QString containing a representation of this longitude
 */
QString Longitude::format (const QString &positiveString, const QString &negativeString) const
{
	unsigned int degrees, minutes, seconds;
	bool positive;
	toDms (degrees, minutes, seconds, positive);

	return qnotrUtf8 ("%1° %2' %3\" %4")
        .arg (degrees)
		.arg (minutes)
		.arg (seconds)
		.arg (positive?positiveString:negativeString);
}

/**
 * Creates a longitude by parsing a string
 *
 * Supported formats:
 *   -     dd mm ss       or       dd° mm' ss"
 *   - +|- dd mm ss       or   +|- dd° mm' ss"
 *   -     dd mm ss E|W   or       dd° mm' ss" E|W
 *
 * If parsing fails, an invalid Longitude is returned.
 *
 * @param string the string to parse. Leading and trailing whitespace is
 *        ignored.
 * @return the parse longitude, or an invalid longitude if the parsing failed
 */
Longitude Longitude::parse (const QString &string)
{
	static const Longitude invalid;

	QString s=string.trimmed ();
	if (s.isEmpty ()) return invalid;

    QRegularExpression re;

	// dd mm ss
	// dd° mm' ss"
    re=QRegularExpression (qnotrUtf8 ("^(\\d+)°?\\s+(\\d+)'?\\s+(\\d+)\"?$"));
    QRegularExpressionMatch match1 = re.match(s);
    if (s.contains (re)) {
        return parse (match1, 1, 2, 3, true);
    }

	// +|- d m s
	// +|- d° m' s"
    re=QRegularExpression (qnotrUtf8 ("^([+-])\\s*(\\d+)°?\\s+(\\d+)'?\\s+(\\d+)\"?$"));
    QRegularExpressionMatch match2 = re.match(s);
    if (match2.hasMatch()) {
        return parse (match2, 2, 3, 4, match2.captured(1)!=notr ("-"));
    }

	// dd mm ss E|W
	// dd° mm' ss" E|W
    re=QRegularExpression (qnotrUtf8 ("^(\\d+)°?\\s+(\\d+)'?\\s+(\\d+)\"?\\s*([EW])$"), QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match3 = re.match(s);
    if (match3.hasMatch()) {
        return parse (match3, 1, 2, 3, match3.captured(4)!=notr ("W") && match3.captured(4)!=notr("w"));
    }

	return invalid;
}

/**
 * Creates a longitude by parsing strings for degrees, minutes and seconds
 *
 * This is a helper method for the parse (const QString &) method.
 */
Longitude Longitude::parse (const QString &degrees, const QString &minutes, const QString &seconds, bool positive)
{
	bool numOk=false;

	int deg=degrees.toUInt (&numOk); if (!numOk) return Longitude ();
	int min=minutes.toUInt (&numOk); if (!numOk) return Longitude ();
	int sec=seconds.toUInt (&numOk); if (!numOk) return Longitude ();

	return Longitude (deg, min, sec, positive);
}

/**
 * Creates a longitude by parsing captured text of a regular expression for
 * degrees, minutes and seconds
 *
 * This is a helper method for the parse (const QString &) method.
 */
Longitude Longitude::parse (const QRegularExpressionMatch &match, int degreesCap, int minutesCap, int secondsCap, bool positive)
{
    return parse (match.captured (degreesCap), match.captured (minutesCap), match.captured (secondsCap), positive);
}
