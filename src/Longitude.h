#ifndef LONGITUDE_H_
#define LONGITUDE_H_

#include "src/accessor.h"

#include <QApplication>
#include <QString>
#include <QTime>

class QRegExp;

/**
 * A longitude
 *
 * The longitude is stored as a double value internally. It can assume
 * arbitrary values, even outside the regular range. Positive values represent
 * eastern longitudes, negative values represent western longitudes.
 */
class Longitude
{
	public:
		// *** Construction
		Longitude ();
		Longitude (double value);
		Longitude (unsigned int degrees, unsigned int minutes, unsigned int seconds, bool positive);
		virtual ~Longitude ();


		// *** Property access
		value_reader (double, Value, value);
		bool_reader (Valid, valid);
		void setValue (double value) { this->value=value; valid=true; }


		// *** Data processing
		Longitude normalized () const;
		double solarTimeOffsetTo (const Longitude &other);
		double clockTimeOffsetTo (const Longitude &other);


		// *** Conversion
		void toDms (unsigned int &degrees, unsigned int &minutes, unsigned int &seconds, bool &positive) const;
		void toDms (unsigned int &degrees, unsigned int &minutes, unsigned int &seconds, bool &positive, double &remainder) const;

		QString format (const QString &eastString=qApp->translate ("Longitude", "E", "east"), const QString &westString=qApp->translate ("Longitude", "W", "west")) const;
		static Longitude parse (const QString &string);

	protected:
		static Longitude parse (const QString &degrees, const QString &minutes, const QString &seconds, bool positive);
        static Longitude parse (const QRegularExpressionMatch &m, int degreesCap, int minutesCap, int secondsCap, bool positive);


	private:
		/** The longitude in degrees */
		double value;
		bool valid;
};

#endif
