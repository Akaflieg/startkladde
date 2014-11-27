/*
 * SunsetPluginBase.cpp
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#include "SunsetPluginBase.h"

#include <QDebug>
#include <QSettings>
#include <QRegExp>
#include <QFile>

#include "SunsetPluginSettingsPane.h"
#include "src/text.h"
#include "src/util/file.h"
#include "src/util/qString.h"
#include "src/util/time.h"
#include "src/config/Settings.h"
#include "src/i18n/notr.h"


// ******************
// ** Construction **
// ******************

SunsetPluginBase::SunsetPluginBase (QString caption, bool enabled, const QString &filename):
	InfoPlugin (caption, enabled),
	filename (filename),
	longitudeCorrection (false)
{
}

SunsetPluginBase::~SunsetPluginBase ()
{
}


// ************************
// ** InfoPlugin methods **
// ************************

PluginSettingsPane *SunsetPluginBase::infoPluginCreateSettingsPane (QWidget *parent)
{
	return new SunsetPluginSettingsPane (this, parent);
}

void SunsetPluginBase::infoPluginReadSettings (const QSettings &settings)
{
	filename=settings.value (notr ("filename"), filename).toString ();
	longitude=Longitude (settings.value (notr ("longitude"), Longitude (9, 27, 0, true).getValue ()).toDouble ());
	longitudeCorrection=settings.value (notr ("longitudeCorrection"), false).toBool ();
}

void SunsetPluginBase::infoPluginWriteSettings (QSettings &settings)
{
	settings.setValue (notr ("filename"), filename);
	settings.setValue (notr ("longitude"), longitude.getValue ());
	settings.setValue (notr ("longitudeCorrection"), longitudeCorrection);
}

QString SunsetPluginBase::configText () const
{
	if (longitudeCorrection)
		return qnotr ("%1, %2").arg (longitude.format (), filename);
	else
		return filename;
}

/**
 * Reads the required data from the data file
 *
 * Resolves the file name. The following data is read:
 *   - the sunset for the current date
 *   - the reference longitude, if longitude correction is activated
 *
 * If the reading fails, the correspondig values are set to invalid
 * (invalid sunset to or referenceLongitude) and an error message
 * is output.
 */
void SunsetPluginBase::start ()
{
	QString filename=getFilename ();

	resolvedFilename=QString ();
	rawSunset=correctedSunset=QTime ();
	referenceLongitude=Longitude ();

	if (isBlank (filename)) OUTPUT_AND_RETURN (tr ("No file specified"));

	resolvedFilename=resolveFilename (filename, Settings::instance ().pluginPaths);
	if (isBlank (resolvedFilename)) OUTPUT_AND_RETURN (tr ("File not found"));
	if (!QFile::exists (resolvedFilename)) OUTPUT_AND_RETURN (tr ("File does not exist"));

	try
	{
		// The file is OK

		// Find the sunset for today
		QString sunsetString=readSunsetString (resolvedFilename);
		if (isBlank (sunsetString)) OUTPUT_AND_RETURN (tr ("Time for current date not found in data file"));

		rawSunset=QTime::fromString (sunsetString, notr ("hh:mm"));
		if (!rawSunset.isValid ()) OUTPUT_AND_RETURN (tr ("Invalid time format"));

		if (longitudeCorrection)
		{
			QString referenceLongitudeString=readReferenceLongitudeString (resolvedFilename);
			if (referenceLongitudeString.isEmpty ()) OUTPUT_AND_RETURN (tr ("No reference longitude found in data file"));

			referenceLongitude=Longitude::parse (referenceLongitudeString);
			if (!referenceLongitude.isValid ()) OUTPUT_AND_RETURN (tr ("Invalid reference longitude"));

			// Add the difference in clock time for the same solar time
			correctedSunset=rawSunset.addSecs (longitude.clockTimeOffsetTo (referenceLongitude));
		}
	}
	catch (FileOpenError &ex)
	{
		outputText (tr ("Error: %1").arg (ex.errorString));
	}
}

/**
 * Discards the values read from the time
 */
void SunsetPluginBase::terminate ()
{
	rawSunset=QTime ();
	correctedSunset=QTime ();
}


// ******************
// ** File reading **
// ******************

/**
 * Reads the sunset from a file as a string
 *
 * The file must contain a line with the current date and the sunset time,
 * separated by whitespace. Example
 *   08-15  18:43
 *
 * If there is no entry for the current date in the file, an empty string is
 * returned.
 *
 * @param filename the file to read from
 * @return the sunset string
 * @throw FileOpenError if the file cannot be opened
 */
QString SunsetPluginBase::readSunsetString (const QString &filename)
{
	QString dateString=QDate::currentDate ().toString (notr ("MM-dd"));
	// TODO does not escape dateString
	QRegExp regexp (qnotr ("^%1\\s*(\\S*)").arg (dateString));

	return findInFile (filename, regexp, 1);
}

/**
 * Reads the reference longitude from a file
 *
 * The file must contain a line with the following format:
 *   ReferenceLongitude: +9 27 00
 *
 * If there is no entry for the reference longitude in the file, an invalid
 * Longitude is returned.
 *
 * @param filename the file to read from
 * @param ok set to true on success or false if there was no reference
 *           longitude entry or it could not be parsed
 * @return the reference longitude
 * @throw FileOpenError if the file cannot be opened
 */
QString SunsetPluginBase::readReferenceLongitudeString (const QString &filename)
{
	return findInFile (filename, QRegExp (notr ("^ReferenceLongitude: (.*)")), 1);
}

/**
 * Reads the source from a file
 *
 * The file must contain a line with the following format:
 *   Source: xyz
 *
 * If there is no entry for the source in the file, an empty
 * string is returned.
 *
 * @param filename the file to read from
 * @return the source
 * @throw FileOpenError if the file cannot be opened
 */
QString SunsetPluginBase::readSource (const QString &filename)
{
	QRegExp regexp (notr ("^Source: (.*)"));

	return findInFile (filename, regexp, 1);
}


// **********
// ** Misc **
// **********

/**
 * Returns the raw or corrected sunset time, depending on the longitude
 * correction setting
 *
 * @return the sunset time to use
 */
QTime SunsetPluginBase::getEffectiveSunset ()
{
	if (longitudeCorrection)
		return getCorrectedSunset ();
	else
		return getRawSunset ();
}
