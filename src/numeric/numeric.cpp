#include <src/numeric/numeric.h>

#include <cmath>

#include <QDebug>

#include "src/numeric/Scientific.h"

/**
 * Rounds a value to the specified number of decimal places (i. e. digits right
 * of the decimal point)
 *
 * Examples:
 *   - roundToDecimalPlaces (  3.456, 0) =>   3
 *   - roundToDecimalPlaces (  3.456, 1) =>   3.4
 *   - roundToDecimalPlaces (  3.456, 2) =>   3.45
 *   - roundToDecimalPlaces ( 23.456, 2) =>  23.45
 *   - roundToDecimalPlaces (123.456, 2) => 123.45
 *   - roundToDecimalPlaces (123    , 2) => 123
 */
double numeric::roundToDecimalPlaces (double value, int places)
{
	double decimalFactor=pow (10, places);
	return round (value*decimalFactor)/decimalFactor;
}

/**
 * Rounds a value to the specified number of total places (i. e. digits left and
 * right of the decimal point)
 *
 * Examples:
 *   - roundToTotalPlaces (  3.456, 1) =>   3
 *   - roundToTotalPlaces (  3.456, 2) =>   3.5
 *   - roundToTotalPlaces (  3.456, 3) =>   3.46
 *   - roundToTotalPlaces ( 23.456, 3) =>  23.5
 *   - roundToTotalPlaces (123.456, 3) => 123
 *   - roundToTotalPlaces (123    , 2) => 120
 */
double numeric::roundToTotalPlaces (double value, int places)
{
	Scientific s (value);
	s.setMantissa (roundToDecimalPlaces (s.mantissa (), places-1));
	return s.toValue ();
}

/**
 * Rounds off decimal places, but conserves at least the specified numer of
 * total places
 *
 * Examples:
 *   - roundToMinimumTotalPlaces (  3.456, 0) =>   3
 *   - roundToMinimumTotalPlaces (  3.456, 1) =>   3
 *   - roundToMinimumTotalPlaces (  3.456, 2) =>   3.5
 *   - roundToMinimumTotalPlaces (  3.456, 3) =>   3.46
 *   - roundToMinimumTotalPlaces ( 23.456, 3) =>  23.4
 *   - roundToMinimumTotalPlaces (123.456, 3) => 123
 *   - roundToMinimumTotalPlaces (123.456, 2) => 123
 *   - roundToMinimumTotalPlaces (123.456, 1) => 123
 *   - roundToMinimumTotalPlaces (123.456, 0) => 123
 */
double numeric::roundToMinimumTotalPlaces (double value, int minimumPlaces)
{
	Scientific s (value);

	int places=qMax (minimumPlaces, s.exponent ()+1);
	s.setMantissa (roundToDecimalPlaces (s.mantissa (), places-1));
	return s.toValue ();
}
