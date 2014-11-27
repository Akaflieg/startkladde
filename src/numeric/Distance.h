#ifndef DISTANCE_H_
#define DISTANCE_H_

#include <QString>

class Distance
{
	public:
		static QString format (double distance, int minimumTotalPlaces);
		static double euclidean (double x, double y);
};

#endif
