#include <src/numeric/Distance.h>

#include <cmath>

#include <QDebug>

#include "src/i18n/notr.h"
#include "src/numeric/numeric.h"

/**
 * Returns m or km
 *
 * @param distance the distance in m
 * @param numDecimalPlaces
 * @return
 */
QString Distance::format (double distance, int minimumTotalPlaces)
{
	QString unit;
	double factor;

	// Determine the unit and the scaling factor
	if (distance<1000) { unit="m" ; factor=1;     }
	else               { unit="km"; factor=0.001; }

	// Scale to the proper unit
	distance*=factor;

	// Round to the specified number of places
	double roundedValue=numeric::roundToMinimumTotalPlaces (distance, minimumTotalPlaces);

	// Construct the string
	return qnotr ("%1 %2").arg (roundedValue).arg (unit);
}

double Distance::euclidean (double x, double y)
{
	return sqrt (x*x+y*y);
}
