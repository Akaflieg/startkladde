#ifndef VELOCITY_H_
#define VELOCITY_H_

/**
 * Defines some constants for velocity calculations
 *
 * All values are in SI units, that is, m/s.
 *
 * Example:
 *   double velocity = 5 * Velocity::km_h; // 5 km/h
 *   double velocity_in_knots = velocity / Velocity::knot; // 2.70
 */
class Velocity
{
	public:
        static constexpr double m_s = 1.0;
        static constexpr double km_h = 1000.0/3600.0; // 1km / 1h = 1000m / 3600s
        static constexpr double knot = 1852.0/3600.0; // 1NM / 1h = 1852m / 3600s
};

#endif
