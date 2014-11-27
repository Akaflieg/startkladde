#ifndef GPRMCSENTENCE_H_
#define GPRMCSENTENCE_H_

#include <QDateTime>

#include "src/nmea/NmeaSentence.h"
#include "src/numeric/GeoPosition.h"

class QString;



/**
 * GPS recommended minimum sentence C
 *
 * Only the timestamp, location and status fields are currently decoded
 */
class GprmcSentence: public NmeaSentence
{
	public:
		GprmcSentence (const QString &line);
		virtual ~GprmcSentence ();

		QDateTime timestamp;
		bool status;
		GeoPosition position;
};

#endif
