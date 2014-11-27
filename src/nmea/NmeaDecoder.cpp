#include "NmeaDecoder.h"

#include "src/io/dataStream/DataStream.h"
#include "src/nmea/Nmea.h"
#include "src/nmea/GprmcSentence.h"
#include "src/nmea/PflaaSentence.h"

NmeaDecoder::NmeaDecoder (QObject *parent): QObject (parent)
{
}

NmeaDecoder::~NmeaDecoder ()
{
}

/**
 * Decodes a line and emits a signal for recognized sentences
 *
 * @param line a complete line, with or without line terminator
 */
void NmeaDecoder::processLine (const QString &line)
{
	if (Nmea::isType ("GPRMC", line))
	{
		GprmcSentence sentence (line);
		if (sentence.isValid ()) emit gprmcSentence (sentence);
	}
	else if (Nmea::isType ("PFLAA", line))
	{
		PflaaSentence sentence (line);
		if (sentence.isValid ()) emit pflaaSentence (sentence);
	}
}

/**
 * Processes a chunk of data
 *
 * The data passed in data is interpreted as UTF8, accumulated, split into lines
 * and passed to the processLine method.
 */
void NmeaDecoder::processData (const QByteArray &data)
{
	static QString buffer;

	// Process the data
	// Example:
	//   f o o \n b a r \n b
	//   0 1 2 3  4 5 6 7  8
	buffer.append (QString::fromUtf8 (data));
	int pos=0;
	while ((pos=buffer.indexOf ("\n"))>=0)
	{
		QString line=buffer.left (pos).trimmed ();
		buffer.remove (0, pos+1);
		processLine (line);
	}
}
