#include <src/numeric/GeoPosition.h>

#include <cmath>
#include <iostream>

#include <QDebug>
#include <QSettings>
#include <QString>

#include "src/util/qString.h"

/**
 * Constructs an invalid GeoPosition
 */
GeoPosition::GeoPosition ()
{
	// Since the two Angle properties are default-constructed, they will be
	// invalid and therefore this GeoPosition will also be invalid.
}

GeoPosition::~GeoPosition ()
{
}

/**
 * Creates a GeoPosition with the given latitude and longitude
 */
GeoPosition::GeoPosition (const Angle &latitude, const Angle &longitude):
	latitude (latitude), longitude (longitude)
{
}

/**
 * Creates a GeoPosition with the given latitude and longitude, specified in
 * degrees.
 *
 * As for precision:
 *   1 degree       = 1e-0 degree = 111 km (60 NM)
 *   0.1 degree     = 1e-1 degree = 11 km
 *   0.01 degree    = 1e-2 degree = 1 km
 *   0.001 degree   = 1e-3 degree = 111 m
 *   0.0001 degree  = 1e-4 degree = 11 m
 *   0.00001 degree = 1e-5 degree = 1 m
 */
GeoPosition GeoPosition::fromDegrees (const double latitude, const double longitude)
{
	return GeoPosition (Angle::fromDegrees (latitude), Angle::fromDegrees (longitude));
}

/**
 * Creates a GeoPosition with the given latitude and longitude, specified in
 * degrees.
 *
 * As for precision:
 *   1 rad          = 1e-0 rad = 6371 km
 *   0.1 rad        = 1e-1 rad =  637 km
 *   0.01 rad       = 1e-2 rad =   64 km
 *   0.001 rad      = 1e-3 rad =    7 km
 *   0.0001 rad     = 1e-4 rad = 6371 m
 *   0.00001 rad    = 1e-5 rad =  637 m
 *   0.000001 rad   = 1e-6 rad =   64 m
 *   0.0000001 rad  = 1e-7 rad =    7 m
 *   0.00000001 rad = 1e-8 rad =  0.7 m
 * @param latitude
 * @param longitude
 * @return
 */
GeoPosition GeoPosition::fromRadians (const double latitude, const double longitude)
{
	return GeoPosition (Angle::fromRadians (latitude), Angle::fromRadians (longitude));
}

/**
 * Calculates the distance (along a great circle) between two points on the
 * earth surface, in meters, given the spherical angle between the points
 *
 * This method can also be used to determine the distance of two points on the
 * same meridian, given their relative latitude, or two points on the equator,
 * given their relative longitude.
 *
 * If the angle is negative, the result will also be negative.
 */
double GeoPosition::greatCircleDistance (const Angle &angle)
{
	return earthRadius * angle.toRadians ();
}

Angle GeoPosition::greatCircleAngle (double distance)
{
	return Angle::fromRadians (distance/earthRadius);
}

/**
 * Calculates the distance between this position and another position
 *
 * The calculation uses an approximation that only works for small differences
 * in latitude.
 *
 * Note that the distance is measured along a path where the direction does not
 * wrap. For example, the distance between 0°N 179°W and 0°N 179°E would be
 * calculated as 358*60 NM, not 2*60 NM which would be more appropriate. This
 * behavior may change in the future, when a great circle distance is used
 * instead.
 *
 * @param reference
 * @return
 */
double GeoPosition::distanceTo (const GeoPosition &reference) const
{
	Angle averageLatitude=(latitude+reference.latitude)/2;
	Angle latitudeDifference =latitude -reference.latitude;
	Angle longitudeDifference=longitude-reference.longitude;

	double east  = earthRadius * longitudeDifference.toRadians () * averageLatitude.cos ();
	double north = earthRadius * latitudeDifference .toRadians ();

	return sqrt (east*east+north*north);
}

/**
 * Calculates the relative position, in meters east and north of a reference
 * position
 *
 * This only works for small differences in latitude. For large differences,
 * the relative position (as given here) is not well-defined because it depends
 * on the path.
 *
 * The east distance will be positive if this position is east of the reference.
 * Likewise, the north distance will be positive if this position is north of
 * the reference.
 *
 * Note that the east direction is defined as the direction where the longitude
 * does not wrap. For example, a point at 179°E would be considered 358° east of
 * a point at 179°W, even though it is more accurately described as 2° west.
 * Also, if the latitude parameters are out of range (±90°), the result will
 * be meaningless.
 *
 * @param reference the other point
 * @return a QPointF with the east distance as x and the north distance as y
 * @see offsetPosition
 */
QPointF GeoPosition::relativePositionTo (const GeoPosition &reference) const
{
	Angle averageLatitude=(latitude+reference.latitude)/2;
	Angle latitudeDifference =latitude -reference.latitude;
	Angle longitudeDifference=longitude-reference.longitude;

	double east  = earthRadius * longitudeDifference.toRadians () * averageLatitude.cos ();
	double north = earthRadius * latitudeDifference .toRadians ();

	return QPointF (east, north);
}

/**
 * Calculates the position that is the given distance (in meters) east and north
 * of the own position
 *
 * This is the inverse of relativePositionTo.
 *
 * @param offset the offset, with the east distance as x and the north distance
 * as y
 * @return the resulting position
 * @see relativePositionTo
 */
GeoPosition GeoPosition::offsetPosition (const QPointF &offset) const
{
	double east =offset.x ();
	double north=offset.y ();

	Angle resultLatitude=latitude+Angle::fromRadians (north/earthRadius);
	Angle averageLatitude=(latitude+resultLatitude)/2;
	Angle resultLongitude=longitude+Angle::fromRadians (east/earthRadius/averageLatitude.cos ());

	return GeoPosition (resultLatitude, resultLongitude);
}

/**
 * Calculates the relative positions of multiple positions with respect to a
 * single comment reference position
 *
 * @param values a QVector of positions
 * @param reference a single position
 * @return a QVector of QPointF, containing one element for each element of
 *         values
 * @see relativePositionsTo (const GeoPosition &)
 */
QVector<QPointF> GeoPosition::relativePositionTo (const QVector<GeoPosition> &values, const GeoPosition &reference)
{
	QVector<QPointF> result (values.size ());

	for (int i=0, n=values.size (); i<n; ++i)
		result[i]=values[i].relativePositionTo (reference);

	return result;
}

/**
 * Returns whether the position is valid
 *
 * Currently, this only checks whether values have been set; no range checking
 * is performed.
 */
bool GeoPosition::isValid () const
{
	// FIXME check the range? With a Latitude and Longitude class, this would
	// be included in this check.
	return latitude.isValid () && longitude.isValid ();
}

/**
 * Outputs a GeoPosition to a Qt debug stream
 */
QDebug operator<< (QDebug dbg, const GeoPosition &position)
{
	dbg.nospace () << "(" << position.getLatitude () << ", " << position.getLongitude () << ")";
	return dbg.space ();
}

/**
 * Stores a QVector of GeoPosition in a QSettings with a given key
 *
 * @see readVector
 */
void GeoPosition::storeVector (QSettings &settings, const QString &key, const QVector<GeoPosition> &vector)
{
	QVariantList valueList;
	foreach (const GeoPosition &point, vector)
		valueList << point.getLongitude ().toDegrees () << point.getLatitude ().toDegrees ();
	settings.setValue (key, valueList);
}

/**
 * Reads a QVector of GeoPosition from a QSettings with a given key
 *
 * @see storeVector
 */
QVector<GeoPosition> GeoPosition::readVector (QSettings &settings, const QString &key)
{
	QVector<GeoPosition> vector;

	QVariantList valueList = settings.value (key).toList ();
	vector.reserve (valueList.size ()/2);
	while (valueList.size ()>=2)
	{
		double lon = valueList.takeFirst ().toDouble ();
		double lat = valueList.takeFirst ().toDouble ();
		vector << GeoPosition::fromDegrees (lat, lon);
	}

	return vector;
}

/**
 * Tests for equality
 *
 * Two invalid GeoPositions are considered equal. A valid and an invalid
 * GeoPosition are not considered equal. Two valid GeoPositions are considered
 * equal if they represent the same position.
 *
 * Note that a GeoPosition includes floating point values, so the equality
 * relation is not to be trusted.
 */
bool GeoPosition::operator== (const GeoPosition &other) const
{
	// Both invalid => equal
	if (!isValid () && !other.isValid ()) return true;

	// Both valid => equal if the values are equal
	if ( isValid () &&  other.isValid ())
		return (latitude==other.latitude) && (longitude==other.longitude);

	// One valid, one invalid => not equal
	return false;
}

/**
 * Tests for inequality
 *
 * Note that a GeoPosition includes floating point values, so the inequality
 * relation is not to be trusted.
 *
 * @see operator==
 */
bool GeoPosition::operator!= (const GeoPosition &other) const
{
	return !(*this==other);
}

/**
 * Calculates a point that lies as far south as the southernmost and as far west
 * as the westernmost of the two points p1 and p2
 *
 * If either or both of the points are invalid, an invalid GeoPosition is
 * returned.
 *
 * This may fail if the longitude wraps.
 */
GeoPosition GeoPosition::southWest (const GeoPosition &p1, const GeoPosition p2)
{
	// If either or both of the latitudes are invalid, Angle::min will return
	// an invalid Angle; the same applies to the longitudes.
	return GeoPosition (
		Angle::min (p1.latitude , p2.latitude ),
		Angle::min (p1.longitude, p2.longitude)
		);
}

/**
 * Calculates a point that lies as far north as the northernmost and as far east
 * as the easternmost of the two points p1 and p2
 *
 * If either or both of the points are invalid, an invalid GeoPosition is
 * returned.
 *
 * This may fail if the longitude wraps.
 */
GeoPosition GeoPosition::northEast (const GeoPosition &p1, const GeoPosition p2)
{
	// If either or both of the latitudes are invalid, Angle::min will return
	// an invalid Angle; the same applies to the longitudes.
	return GeoPosition (
		Angle::max (p1.latitude , p2.latitude ),
		Angle::max (p1.longitude, p2.longitude)
		);
}
