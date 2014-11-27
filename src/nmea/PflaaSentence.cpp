#include "PflaaSentence.h"

#include <QStringList>

#include "src/nmea/Nmea.h"

PflaaSentence::PflaaSentence (const QString &line):
	NmeaSentence (line, "PFLAA", 12),
	relativeNorth (0), relativeEast (0), relativeVertical (0),
	groundSpeed (0), climbRate (0)
{
	// $PFLAA,0,-52,-11,-4,2,DDAFD0,0,,0,0.1,1*07

	if (!isValid ()) return;
	QStringList parts = this->parts ();

	bool ok;

	// parts[1]: alarm level

	// parts[2]: relative north
	relativeNorth = parts[2].toInt (&ok);
	if (!ok) setValid (false);

	// parts[3]: relative east
	relativeEast = parts[3].toInt (&ok);
	if (!ok) setValid (false);

	// parts[4]: relative vertical
	relativeVertical = parts[4].toInt (&ok);
	if (!ok) setValid (false);

	// parts[5]: ID type

	// parts[6]: Flarm ID
	flarmId = parts[6];

	// parts[7]: true track in degrees

	// parts[8]: right turn rate in deg/s (currently omitted)

	// parts[9]: ground speed
	groundSpeed = parts[9].toInt (&ok);
	if (!ok) setValid (false);

	// parts[10]: climb rate
	climbRate = parts[10].toDouble (&ok);
	if (!ok) setValid (false);

	// parts[11]: aircraft type
}

PflaaSentence::~PflaaSentence ()
{
}
