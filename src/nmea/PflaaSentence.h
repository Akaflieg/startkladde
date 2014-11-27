#ifndef PFLAASENTENCE_H_
#define PFLAASENTENCE_H_

#include <QString>

#include "src/nmea/NmeaSentence.h"

/**
 * Flarm aircraft data
 *
 * This sentence is specified in the Flarm data port specifications,
 * http://www.flarm.com/support/manual/FLARM_DataportManual_v6.00E.pdf
 *
 * Not all fields are currently parsed.
 */
class PflaaSentence: public NmeaSentence
{
	public:
		PflaaSentence (const QString &line);
		virtual ~PflaaSentence ();

		int relativeNorth;    // true in meters
		int relativeEast;     // true in meters
		int relativeVertical; // above in meters
		QString flarmId;
		int groundSpeed;      // in m/s
		double climbRate;     // in m/s
};

#endif
