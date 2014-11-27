#ifndef SunsetPluginBase_H_
#define SunsetPluginBase_H_

#include <QTime>

#include "src/plugin/info/InfoPlugin.h"
#include "src/Longitude.h"
#include "src/i18n/notr.h"

/**
 * A plugin which displays the sunset time for the current date, read from a
 * data file
 *
 * Settings:
 *  - filename: the name of the file to read the data from
 */
class SunsetPluginBase: public InfoPlugin
{
		Q_OBJECT

	public:
		friend class SunsetPluginSettingsPane;

		SunsetPluginBase (QString caption=QString (), bool enabled=true, const QString &filename=notr ("sunsets.txt"));
		virtual ~SunsetPluginBase ();

		virtual PluginSettingsPane *infoPluginCreateSettingsPane (QWidget *parent=NULL);

		virtual void infoPluginReadSettings (const QSettings &settings);
		virtual void infoPluginWriteSettings (QSettings &settings);

		value_accessor (QString, Filename, filename);
		value_accessor (Longitude, Longitude, longitude);
		value_accessor (bool, LongitudeCorrection, longitudeCorrection);

		virtual QString configText () const;

		virtual void start ();
		virtual void terminate ();


		static QString readSunsetString (const QString &filename);
		static QString readSource (const QString &filename);
		static QString readReferenceLongitudeString (const QString &filename);

	protected:
		value_reader (QTime, RawSunset, rawSunset);
		value_reader (QTime, CorrectedSunset, correctedSunset);
		virtual QTime getEffectiveSunset ();

	private:
		// Settings
		QString filename;
		bool longitudeCorrection;
		Longitude longitude;

		// Runtime data
		QString resolvedFilename;
		Longitude referenceLongitude;
		QTime rawSunset; // UTC
		QTime correctedSunset; // UTC
};

#endif
