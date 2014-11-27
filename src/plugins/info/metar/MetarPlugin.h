#ifndef METARPLUGIN_H_
#define METARPLUGIN_H_

#include "src/plugin/info/InfoPlugin.h"

#include <QNetworkReply> // Required for QNetworkReply::NetworkError
#include <QTimer>

class Downloader;

/**
 * An info plugin which displays METAR messages downloaded from the internet
 *
 * Settings:
 *  - airport: the ICAO code of the airport to display the METAR of
 *  - refreshInterval: the interval for refreshing METAR messages in minutes
 */
class MetarPlugin: public InfoPlugin
{
		Q_OBJECT
		SK_PLUGIN

	public:
		// TODO plugins should have enabled as last parameter and no default
		// for others. Problem: InfoPlugin::DefaultDescriptor wants to
		// default-construct plugins.
		MetarPlugin (const QString &caption=QString (), bool enabled=true, const QString &airport="", int refreshInterval=15*60);
		virtual ~MetarPlugin ();

		virtual void start ();
		virtual void terminate ();

		virtual PluginSettingsPane *infoPluginCreateSettingsPane (QWidget *parent=NULL);

		virtual void infoPluginReadSettings (const QSettings &settings);
		virtual void infoPluginWriteSettings (QSettings &settings);

		virtual QString configText () const;

		value_accessor (QString, Airport        , airport        );
		value_accessor (int    , RefreshInterval, refreshInterval);

	private:
		QString airport;
		int refreshInterval; // seconds
		QTimer *timer;

		Downloader *downloader;

		QString extractMetar (QIODevice &reply);

	public slots:
		void downloadSucceeded (int state, QNetworkReply *reply);
		void downloadFailed    (int state, QNetworkReply *reply, QNetworkReply::NetworkError code);

	protected slots:
		void languageChanged ();

	private slots:
		void refresh ();
};

#endif
