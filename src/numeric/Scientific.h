#ifndef SCIENTIFIC_H_
#define SCIENTIFIC_H_

/**
 * Scientific representation of floating point numbers
 *
 * The number is represented as (mantissa * 10 ^ exponent), where the mantissa
 * is a (positive or negative) floating point number and the exponent is a
 * signed integer.
 *
 * The number is said to be "normalized" if (1 <= mantissa < 10) for non-zero
 * values, or (mantissa = exponent = 0) for zero values.
 */
class Scientific
{
	public:
		Scientific (double mantissa, int exponent);
		Scientific (double value);
		virtual ~Scientific ();

		double mantissa () const;
		int    exponent () const;

		void setMantissa (double mantissa);
		void setExponent (int    exponent);

		void setValue (double value);
		double toValue ();

	private:
		double _mantissa;
		int _exponent;
};

#endif
