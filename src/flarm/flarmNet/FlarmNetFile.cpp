#include <src/flarm/flarmNet/FlarmNetFile.h>

#include <QDebug>
#include <QRegExp>

// According to XCSoar's FlarmNetReader.cpp, FlarmNet files are "ISO-Latin-1,
// which is kind of short-sighted". The number in the first line seems to be a
// release version number which is incremented on each change.


// **********************
// ** Individual lines **
// **********************

// Example:
// 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3 3 3 3 3 4 4 4 4 4 4 4 4 4 4 5 5 5 5 5 5 5 5 5 5 6 6 6 6 6 6 6 6 6 6 7 7 7 7 7 7 7 7 7 7 8 8 8 8 8 8
// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// 4646463031314653562d4765727374657474656e20202020202020454450542020202020202020202020202020202020526f62696e20445220343030202020202020202020442d45415246205246203132332e303030
// F F F 0 1 1 F S V - G e r s t e t t e n               E D P T                                   R o b i n   D R   4 0 0                   D - E A R F   R F   1 2 3 . 0 0 0
// FlarmID-----Owner-------------------------------------                                          Type--------------------------------------Registration--CS----Frequency-----

/**
 * Decodes an encoded line to a plain line
 */
QString FlarmNetFile::decodeLine (const QString &encodedLine)
{
	QString result;

	int lineLength=encodedLine.length ();

	// Iterate over the characters of the line in increments of 2
	for (int i=0; i<lineLength-1; i+=2)
	{
		QString substring=encodedLine.mid (i, 2);

		bool ok=false;
		int character=substring.toInt (&ok, 16);

		if (ok)
			result.append (QChar (character));
	}

	return result;
}

/**
 * Encodes a plain line to an encoded line
 */
QString FlarmNetFile::encodeLine (const QString &plainLine)
{
	QString result;

	// Convert the string to latin1
	QByteArray data=plainLine.toLatin1 ();

	// Iterate over the characters
	for (int i=0; i<data.length (); ++i)
	{
		uchar value=data[i];

		QString byteString=QString::number (value, 16);
		if (result.size ()==1)
			result.append ("0"+byteString);
		else
			result.append (byteString);
	}

	return result.toLower ();
}

// ****************
// ** Line lists **
// ****************

/**
 * Decodes a list of encoded lines to a list of plain lines using decodeLine()
 */
QStringList FlarmNetFile::decodeLines (const QStringList &encodedLines)
{
	// TODO use QtConcurrent::blockingMapped?
	QStringList result;

	foreach (const QString &line, encodedLines)
		result.append (decodeLine (line));

	return result;
}

/**
 * Encodes a list of plain lines to a list of encoded lines using encodeLine()
 */
QStringList FlarmNetFile::encodeLines (const QStringList &plainLines)
{
	QStringList result;

	foreach (const QString &line, plainLines)
		result.append (encodeLine (line));

	return result;
}


// ***************************
// ** Whole files (encoded) **
// ***************************

/**
 * Decodes an encoded file to a list of plain lines
 */
QStringList FlarmNetFile::decodeFile (const QString &encodedFile)
{
	// Split the data into lines (also split on \r for robustness)
	QStringList encodedLines=encodedFile.split (QRegExp ("[\r\n]"), QString::SkipEmptyParts);

	// Ignore the header line
	encodedLines.removeFirst ();

	// Decode the (remaining) lines
	return decodeLines (encodedLines);
}

/**
 * Encodes a list of plain lines to an encoded file
 *
 * Note that the header is written as "000000".
 */
QString FlarmNetFile::encodeFile (const QStringList &lines)
{
	// Encode the lines
	QStringList encodedLines=encodeLines (lines);

	encodedLines.prepend ("000000");

	// Concatenate the lines
	return encodedLines.join ("\n");
}


// **********************************
// ** Whole files (as byte arrays) **
// **********************************

/**
 * Decodes a raw file to a list of plain strings
 */
QStringList FlarmNetFile::decodeRawFile (const QByteArray &rawFile)
{
	QString encodedFile=QString::fromLatin1 (rawFile.constData (), rawFile.size ());
	return decodeFile (encodedFile);
}

/**
 * Encodes a list of plain strings to a raw file
 */
QByteArray FlarmNetFile::encodeFileRaw (const QStringList &lines)
{
	QString string=encodeFile (lines);
	return string.toLatin1 ();
}


// **********************
// ** FlarmNet records **
// **********************

/**
 * Creates a FlarmNet record from a plain line and returns it as value
 *
 * If ok is not NULL, it will be set to true if the line is valid or false if
 * the line is invalid. The return value is undefined if the line is invalid.
 */
FlarmNetRecord FlarmNetFile::createRecord (const QString &plainLine, bool *ok)
{
	FlarmNetRecord record;

	// Initialize the OK flag to false. It will be set to true when the line is
	// valid.
	bool isOk=false;

	if (plainLine.length ()==86)
	{
		record.flarmId     =(plainLine.mid ( 0,  6).trimmed ());
		record.owner       =(plainLine.mid ( 6, 21).trimmed ());
		record.airfield    =(plainLine.mid (27, 21).trimmed ());
		record.type        =(plainLine.mid (48, 20).trimmed ());
		record.registration=(plainLine.mid (69,  7).trimmed ());
		record.callsign    =(plainLine.mid (76,  3).trimmed ());
		record.frequency   =(plainLine.mid (79,  7).trimmed ());

		// Perform some basic validity checks and set the OK flag on success
		if (record.flarmId.length ()==6 && record.registration.length ()>0)
			isOk=true;
	}

	if (ok)
		(*ok)=isOk;

	return record;
}

/**
 * Creates a list of FlarmNet records from a raw file
 *
 * numGood and numBad are written with the number of successfully and
 * unsuccessfully created records, respectively, unless they are NULL.
 */
QList<FlarmNetRecord> FlarmNetFile::createRecords (const QByteArray &rawFile, int *numGood, int *numBad)
{
	QList<FlarmNetRecord> result;

	// Initialize the counters for good and bad lines
	int good=0;
	int bad=0;

	// Create a record for each line and count the good and bad lines
	QStringList plainLines=decodeRawFile (rawFile);
	foreach (const QString &plainLine, plainLines)
	{
		// Create a record from the line
		bool ok=false;
		FlarmNetRecord record=createRecord (plainLine, &ok);

		if (ok)
		{
			++good;
			result.append (record);
		}
		else
		{
			++bad;
		}
	}

	// Write back the good and bad counts
	if (numGood) (*numGood)=good;
	if (numBad ) (*numBad )=bad ;

	return result;
}
