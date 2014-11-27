#include "GprmcSentence.h"

#include <QStringList>

#include "src/nmea/Nmea.h"
#include "src/nmea/NmeaSentence.h"

GprmcSentence::GprmcSentence (const QString &line):
	NmeaSentence (line, "GPRMC", 13),
	status (false)
{
	// $GPRMC,103400.00,A,5256.58562,N,01247.34325,E,0.002,,100911,,,A*77

	if (!isValid ()) return;
	QStringList parts = this->parts ();

	// parts[1,9]: time and date
	timestamp = Nmea::parseDateTime (parts[9], parts[1]);

	// parts[2]: status flag
	status = parts[2]=="A";

	// parts[3,4,5,6]: latitude value and sign, longitude value and sign
	Angle latitude  = Nmea::parseLatitude  (parts[3], parts[4]);
	Angle longitude = Nmea::parseLongitude (parts[5], parts[6]);
	position=GeoPosition (latitude, longitude);

	// parts[7]: velocity

	// parts[8]: true track in degrees

	// parts[9]: date (already used)

	// parts[10]: variation

	// parts[11]: variationSign

	// parts[12]: integrity
}

GprmcSentence::~GprmcSentence ()
{
}
