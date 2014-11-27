#include "src/nmea/NmeaSentence.h"

#include <iostream>

#include "src/nmea/Nmea.h"
#include "src/util/qString.h"

/**
 * Creates an NMEA sentence
 *
 * @param line the line of data as received from the GPS device. Leading and
 *             trailing whitespace is ignored.
 * @param expectedType the expected sentence type, without the leading dollar
 *                     sign; for example, "GPRMC". The sentence will be
 *                     considered invalid if the actual sentence type does not
 *                     match the expected sentence type.
 * @param expectedNumberOfParts the expected minimum number of parts, including
 *                              the sentence type. The sentence will be
 *                              considered invalid if the actual number of parts
 *                              is different from the expected number.
 */
NmeaSentence::NmeaSentence (const QString &line, const QString &expectedType, int expectedNumberOfParts):
	_line (line.trimmed ()), _valid (false)
{
	// $GPRMC,103400.00,A,5256.58562,N,01247.34325,E,0.002,,100911,,,A*77

	// Remove whitespace from the beginning and end of the string
	int len=_line.length ();

	// The string must be long enough to contain the $ and the checksum.
	if (len<4) return;

	// The string must begin with a $ and contain a * at the third place from
	// the end
	if (_line[0]    .toAscii ()!='$') return;
	if (_line[len-3].toAscii ()!='*') return;

	// Extract the data part and the checksum
	QString data    =_line.mid (1    , len-4);
	QString checksum=_line.mid (len-2, 2    );

	// The extracted checksum must match the calculated checksum
	uint8_t calculatedChecksum=Nmea::calculateChecksum (data);
	uint8_t extractedChecksum=checksum.toUShort (NULL, 16);
	if (calculatedChecksum!=extractedChecksum) return;

	// Extract the parts
	_parts=data.split (',');

	// The number of parts must match the expected value
	if (_parts.size ()!=expectedNumberOfParts) return;

	// The type must match the expected type
	if (_parts[0]!=expectedType) return;

	// Everything seems to be in order
	_valid=true;
}

NmeaSentence::~NmeaSentence ()
{
}

/**
 * Returns the original received line, as passed to the constructor
 */
QString NmeaSentence::line () const
{
	return _line;
}

/**
 * Returns true if the sentence is considered valid
 */
bool NmeaSentence::isValid () const
{
	return _valid;
}

/**
 * Returns a list of parts (separated by commas in the received line)
 *
 * This list includes the sentence type part (e. g. "GPRMC") as item 0.
 */
QStringList NmeaSentence::parts () const
{
	return _parts;
}

/**
 * Returns the sentence type (e. g. "GPRMC")
 */
QString NmeaSentence::type () const
{
	return _parts[0];
}

/**
 * Sets the valid flag to the specified value
 */
void NmeaSentence::setValid (bool valid)
{
	_valid=valid;
}
