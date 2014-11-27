#include "Angle.h"

#include <cmath>

#include <QPointF>

#include "src/i18n/notr.h"

/**
 * Constructs an invalid angle
 */
Angle::Angle ():
	value (0), valid (false)
{
}

/**
 * Constructs a valid angle with the given value
 *
 * This method is private.
 *
 * @param value the angle value, in radians
 */
Angle::Angle (double value):
	value (value), valid (true)
{
}

/**
 * Constructs an angle with the given value and validity
 *
 * This method is private.
 *
 * @param value the value, in radians
 * @param valid true for a valid angle, false else
 */
Angle::Angle (double value, bool valid):
	value (value), valid (valid)
{
}

Angle::~Angle ()
{
}

/**
 * Constructs an angle with a given value in radians
 *
 * @param radians the value, in radians
 * @return an Angle with the given value
 */
Angle Angle::fromRadians (double radians)
{
	return Angle (radians);
}

/**
 * Constructs an angle with a given value in degrees
 *
 * @param degrees the value, in degrees
 * @return an Angle with the given value
 */
Angle Angle::fromDegrees (double degrees)
{
	return Angle (degrees/360*(2*M_PI));
}

/**
 * Constructs an angle with a given value in arc minutes
 *
 * @param minutes the value, in arc minutes
 * @return an Angle with the given value
 */
Angle Angle::fromMinutes (double minutes)
{
	return Angle (minutes/(360*60)*(2*M_PI));
}

/**
 * Creates an angle spanning a full circle (360 degrees or 2*pi radians)
 */
Angle Angle::fullCircle ()
{
	return Angle::fromRadians (2*M_PI);
}

/**
 * Creates a zero angle
 */
Angle Angle::zero ()
{
	return Angle::fromRadians (0);
}

/**
 * Returns the status of the valid flag
 */
bool Angle::isValid () const
{
	return valid;
}

/**
 * Returns the value, in radians
 *
 * If the Angle is invalid, the result is undefined.
 */
double Angle::toRadians () const
{
	return value;
}

/**
 * Returns the value, in degrees
 *
 * If the Angle is invalid, the result is undefined.
 */
double Angle::toDegrees () const
{
	return value/(2*M_PI)*360;
}

/**
 * Returns the value, in arc minutes
 *
 * If the Angle is invalid, the result is undefined.
 */
double Angle::toMinutes () const
{
	return value/(2*M_PI)*360*60;
}

/**
 * Returns an Angle equivalent to this one, but normalized to the range from 0°
 * (included) to 360° (not included)
 */
Angle Angle::normalized () const
{
	double v=toRadians ();
	while (v>=2*M_PI) v-=2*M_PI;
	while (v<0      ) v+=2*M_PI;
	return Angle::fromRadians (v);
}

/**
 * Returns the sum of this and other
 */
Angle Angle::operator+ (const Angle &other) const
{
	if (valid && other.valid)
		return Angle (value+other.value);
	else
		return Angle ();
}

/**
 * Returns the difference between this and other
 */
Angle Angle::operator- (const Angle &other) const
{
	if (valid && other.valid)
		return Angle (value-other.value);
	else
		return Angle ();
}

/**
 * Returns the result of multiplying this angle with the scalar factor
 */
Angle Angle::operator* (double factor) const
{
	return Angle (value*factor, valid);
}

/**
 * Returns the result of dividing this angle by the scalar factor
 */
Angle Angle::operator/ (double factor) const
{
	return Angle (value/factor, valid);
}

double Angle::operator/ (const Angle &other) const
{
	return value/other.value;
}

/**
 * Returns the negated angle
 */
Angle Angle::operator- () const
{
	return Angle (-value, valid);
}

/**
 * Tests for equality
 *
 * Two invalid Angles are considered equal. A valid and an invalid Angle are not
 * considered equal. Two valid Angles are considered equal if they represent the
 * same angle.
 *
 * Note that an angle is stored as a floating point value, so the equality
 * relation is not to be trusted.
 */
bool Angle::operator== (const Angle &other) const
{
	// Both invalid => equal
	if (!valid && !other.valid) return true;

	// Both valid => equal if the values are equal
	if ( valid &&  other.valid) return value==other.value;

	// One valid, one invalid => not equal
	return false;
}

/**
 * Tests for inequality
 *
 * Note that an angle is stored as a floating point value, so the inequality
 * relation is not to be trusted.
 *
 * @see operator==
 */
bool Angle::operator!= (const Angle &other) const
{
	return !(*this==other);
}

/**
 * Tests for less-than
 *
 * If either or both of the angles are invalid, this operator returns false.
 */
bool Angle::operator< (const Angle &other) const
{
	if (!valid || !other.valid) return false;
	return value<other.value;
}

/**
 * Tests for less-or-equal-than
 *
 * If either or both of the angles are invalid, this operator returns false.
 */
bool Angle::operator<= (const Angle &other) const
{
	if (!valid || !other.valid) return false;
	return value<=other.value;
}

/**
 * Tests for greater-then
 *
 * If either or both of the angles are invalid, this operator returns false.
 */
bool Angle::operator> (const Angle &other) const
{
	if (!valid || !other.valid) return false;
	return value>other.value;
}

/**
 * Tests for greater-or-equal-than
 *
 * If either or both of the angles are invalid, this operator returns false.
 */
bool Angle::operator>= (const Angle &other) const
{
	if (!valid || !other.valid) return false;
	return value>=other.value;
}

/**
 * Returns the smaller of two Angles
 *
 * If either or both of the angles are invalid, an invalid angle is returned.
 */
Angle Angle::min (const Angle &a1, const Angle &a2)
{
	if (a1.valid && a2.valid)
		return (a1.value<a2.value) ? a1.value : a2.value;
	else
		return Angle ();
}

/**
 * Returns the greater of two Angles
 *
 * If either or both of the angles are invalid, an invalid angle is returned.
 */
Angle Angle::max (const Angle &a1, const Angle &a2)
{
	if (a1.valid && a2.valid)
		return (a1.value>a2.value) ? a1.value : a2.value;
	else
		return Angle ();
}

/**
 * Returns an Angle constructed by calling the atan2 function
 *
 * If both x and y are 0, an invalid Angle is returned.
 */
Angle Angle::atan2 (const double y, const double x)
{
	if (x==0 && y==0)
		return Angle ();
	else
		return Angle::fromRadians (::atan2 (y, x));
}

/**
 * Returns an Angle constructed by calling the atan2 function
 *
 * If the point is (0, 0), an invalid Angle is returned.
 */
Angle Angle::atan2 (const QPointF &point)
{
	return Angle::atan2 (point.y (), point.x ());
}

/**
 * Divides the full circle into numSections sections of equal size and returns
 * the number of the section containing the (normalized) Angle.
 *
 * For example, for numSections=4, the sections are as follows:
 *     0  <= Angle <  45°: section 0 (north)
 *    45° <= Angle < 135°: section 1 (east)
 *   135° <= Angle < 225°: section 2 (south)
 *   225° <= Angle < 315°: section 3 (west)
 *   315° <= Angle < 360°: section 0 (north)
 *
 * For other values of numSections, the sections are placed such that section 0
 * is centered at 0° (north).
 *
 * @param numSections the number of sections. Typical values are 4 or 8.
 * @return an integer representing the section, with 0 <= section < numSections
 */
int Angle::compassSection (int numSections) const
{
	// Convert to a range from 0 (0 degrees) to numSections (360 degrees)
	double v=normalized ().toRadians ()/(2*M_PI)*numSections;

	// Round to the closest integer
	int n=round (v);

	// The section adjacent to 360 degrees should be 0, not numSections
	if (n==numSections)
		n=0;

	return n;
}

/**
 * Outputs the Angle to a QDebug stream
 */
QDebug operator<< (QDebug dbg, const Angle &angle)
{
	if (angle.isValid ())
		dbg.nospace() << angle.toDegrees () << "°";
	else
		dbg.nospace () << "invalid angle";

    return dbg.space();
}

double Angle::sin () const
{
	return std::sin (value);
}

double Angle::cos () const
{
	return std::cos (value);
}

double Angle::tan () const
{
	return std::tan (value);
}

QString Angle::formatDmSuffix (const QString &positiveSuffix, const QString &negativeSuffix) const
{
	double degrees=toDegrees ();

	bool positive=(degrees>0);
	int minutes=round (fabs (degrees)*60);

	return qnotrUtf8 ("%1° %2' %3")
		.arg (minutes/60)
		.arg (minutes%60)
		.arg (positive ? positiveSuffix : negativeSuffix);
}
