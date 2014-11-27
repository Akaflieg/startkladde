#ifndef ANGLE_H_
#define ANGLE_H_

#include <QDebug>
#include <QString>

class QPointF;

/**
 * FIXME merge with Longitude class
 */
/**
 * A class to store an angle value
 *
 * The angle can be initialized and converted to degrees and radians. An
 * additional flag indicates whether the angle is valid.
 *
 * Internally, the value is stored in radians. This is not visible on the
 * interface, but initializing from and converting to radians will be slightly
 * faster than for degrees.
 */
class Angle
{
	public:
		Angle ();
		virtual ~Angle ();

		static Angle fromRadians (double radians);
		static Angle fromDegrees (double degrees);
		static Angle fromMinutes (double minutes);
		//static Angle fromDMS (bool positive, uint degrees, uint minutes, uint seconds);
		//static Angle fromDMS_asymmetric (int degrees, uint minutes, uint seconds);
		static Angle zero ();
		static Angle fullCircle ();

		Angle normalized () const;

		bool isValid () const;

		double toRadians () const;
		double toDegrees () const;
		double toMinutes () const;
		//void toDMS (bool *positive, uint *degrees, uint *minutes, uint *seconds) const;
		//void toDMS_asymmetric (int *degrees, uint *minutes, uint *seconds) const;

		Angle operator+ (const Angle &other) const;
		Angle operator- (const Angle &other) const;
		Angle operator* (double factor) const;
		Angle operator/ (double factor) const;
		double operator/ (const Angle &other) const;

		Angle operator- () const;

		bool operator== (const Angle &other) const;
		bool operator!= (const Angle &other) const;
		bool operator<  (const Angle &other) const;
		bool operator<= (const Angle &other) const;
		bool operator>  (const Angle &other) const;
		bool operator>= (const Angle &other) const;

		double sin () const;
		double cos () const;
		double tan () const;

		static Angle min (const Angle &a1, const Angle &a2);
		static Angle max (const Angle &a1, const Angle &a2);

		static Angle atan2 (const double y, const double x);
		static Angle atan2 (const QPointF &point);

		int compassSection (int numSections) const;

		QString formatDmSuffix (const QString &positiveSuffix, const QString &negativeSuffix) const;

	private:
		Angle (double value);
		Angle (double value, bool valid);

		double value;
		bool valid;
};

QDebug operator<< (QDebug dbg, const Angle &angle);

#endif
