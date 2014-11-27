/*
 * MetarPlugin.cpp
 *
 *  Created on: 07.07.2010
 *      Author: Martin Herrmann
 */

#include "MetarPlugin.h"

//#include <QDebug>
#include <QSettings>
#include <QRegExp>
#include <QString>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include "src/plugin/factory/PluginFactory.h"
#include "MetarPluginSettingsPane.h"
#include "src/util/qString.h"
#include "src/util/io.h"
#include "src/net/Downloader.h"
#include "src/i18n/notr.h"

REGISTER_PLUGIN (InfoPlugin, MetarPlugin)
SK_PLUGIN_DEFINITION (
	MetarPlugin,
	notr ("{4a6c7218-42ae-475d-8fd9-a2a131c1aa90}"),
	MetarPlugin::tr ("METAR"),
	MetarPlugin::tr ("Displays METAR messages (internet connection required)"))

MetarPlugin::MetarPlugin (const QString &caption, bool enabled, const QString &airport, int refreshInterval):
	InfoPlugin (caption, enabled),
	airport (airport), refreshInterval (refreshInterval),
	timer (new QTimer (this)), downloader (new Downloader (this))
{
	downloader->connectSignals (this);
	connect (timer, SIGNAL (timeout ()), this, SLOT (refresh ()));
}

MetarPlugin::~MetarPlugin ()
{
}

PluginSettingsPane *MetarPlugin::infoPluginCreateSettingsPane (QWidget *parent)
{
	return new MetarPluginSettingsPane (this, parent);
}

void MetarPlugin::infoPluginReadSettings (const QSettings &settings)
{
	airport         =settings.value (notr ("airport")        , airport        ).toString ();
	refreshInterval =settings.value (notr ("refreshInterval"), refreshInterval).toInt    ();
}

void MetarPlugin::infoPluginWriteSettings (QSettings &settings)
{
	settings.setValue (notr ("airport")        , airport        );
	settings.setValue (notr ("refreshInterval"), refreshInterval);
}

void MetarPlugin::start ()
{
	refresh ();

	if (timer->isActive ()) timer->stop ();
	timer->start (1000*refreshInterval);
}

void MetarPlugin::terminate ()
{
	downloader->abort ();
	timer->stop ();
}

QString MetarPlugin::configText () const
{
	return tr ("%1 (%2 minutes)").arg (airport).arg (refreshInterval/60);
}

void MetarPlugin::refresh ()
{
	QString icao=airport.trimmed ().toUpper ();

	if (icao.isEmpty ())
		outputText (tr ("No airport specified"));
	if (!airport.contains (QRegExp (notr ("^[A-Z]{4,4}$"))))
		outputText (tr ("%1 is not a valid ICAO code").arg (airport));
	else
	{
		QString url=qnotr ("http://weather.noaa.gov/mgetmetar.php?cccc=%1").arg (icao);
		outputText (tr ("Retrieving METAR for %1...").arg (icao));
		downloader->startDownload (0, url);
	}
}

QString MetarPlugin::extractMetar (QIODevice &reply)
{
	// The relevant section of the page:
	//
	//<font face="courier" size = "5">
	//EDDF 311920Z 26009KT 9999 FEW012 SCT019 BKN023 13/11 Q1015 NOSIG
	//
	//</font>
	QString re=qnotr ("^%1.*$").arg (airport.trimmed ().toUpper ());
	return findInIoDevice (reply, QRegExp (re), 0);
}

void MetarPlugin::downloadSucceeded (int state, QNetworkReply *reply)
{
	(void)state; // There is only one download

	QString metar=extractMetar (*reply);

	if (metar.isEmpty ())
		outputText (tr ("Error: no METAR found for %1").arg (airport));
	else
		outputText (metar);
}

void MetarPlugin::downloadFailed (int state, QNetworkReply *reply, QNetworkReply::NetworkError code)
{
	(void)state; // There is only one download

	if (code==QNetworkReply::ContentNotFoundError)
		outputText (tr ("Error: METAR page not found (404)"));
	else
		outputText (reply->errorString ());
}

void MetarPlugin::languageChanged ()
{
	// Nothing to do
}
