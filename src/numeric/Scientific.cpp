#include "src/numeric/Scientific.h"

#include <cmath>


// ******************
// ** Construction **
// ******************

/**
 * Creates a Scientific number with a given mantissa and exponent
 *
 * The number is not necessarily normalized.
 */
Scientific::Scientific (double mantissa, int exponent):
	_mantissa (mantissa), _exponent (exponent)
{
}

/**
 * Creates a Scientific number with a given value. The number will be
 * normalized.
 */
Scientific::Scientific (double value)
{
	setValue (value);
}

Scientific::~Scientific ()
{
}


// ****************
// ** Properties **
// ****************

/**
 * Returns the mantissa of the number (not necessarily normalized)
 */
double Scientific::mantissa () const
{
	return _mantissa;
}

/**
 * Returns the exponent of the number (not necessarily normalized)
 */
int Scientific::exponent () const
{
	return _exponent;
}

/**
 * Sets the mantissa of the number. The exponent is not changed. The value may
 * change. The number is not necessarily normalized afterwards.
 */
void Scientific::setMantissa (double mantissa)
{
	_mantissa=mantissa;
}

/**
 * Sets the exponent of the number. The mantissa is not changed. The value may
 * change. The number is not necessarily normalized afterwards.
 */
void Scientific::setExponent (int exponent)
{
	_exponent=exponent;
}


// ****************
// ** Conversion **
// ****************

/**
 * Sets the number to a value. The number will be normalized.
 */
void Scientific::setValue (double value)
{
	if (value==0)
	{
		_mantissa=0;
		_exponent=0;
	}
	else
	{
		// Calculate exponent and mantissa
		_exponent=floor (log (fabs (value))/log (10));
		_mantissa=value/pow (10, _exponent);
	}
}

/**
 * Converts the number to a value
 */
double Scientific::toValue ()
{
	return _mantissa * pow (10, _exponent);
}
