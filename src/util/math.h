#ifndef MATH_H_
#define MATH_H_

/**
 * Returns the value if it is positive, or 0 otherwise
 *
 * Basically, this is the same as qMax (value, 0).
 */
template <typename T> inline const T ifPositive (const T &value)
{
	if (value>0)
		return value;
	else
		return 0;
}

#endif
