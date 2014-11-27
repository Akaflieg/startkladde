#ifndef NUMERIC_H_
#define NUMERIC_H_

namespace numeric
{
	double roundToDecimalPlaces (double value, int places);
	double roundToTotalPlaces (double value, int places);
	double roundToMinimumTotalPlaces (double value, int minimumPlaces);
}

#endif
