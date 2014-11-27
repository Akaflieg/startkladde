/*
 * WetterOnlineAnimationPlugin.cpp
 *
 *  Created on: 25.07.2010
 *      Author: martin
 */

#include "WetterOnlineAnimationPlugin.h"

#include <QMovie>
#include <QRegExp>

#include "src/plugin/factory/PluginFactory.h"
#include "src/net/Downloader.h"
#include "src/util/qString.h"
#include "src/util/io.h"
#include "src/text.h"
#include "src/graphics/SkMovie.h"
#include "src/i18n/notr.h"

REGISTER_PLUGIN (WeatherPlugin, WetterOnlineAnimationPlugin)
SK_PLUGIN_DEFINITION (
	WetterOnlineAnimationPlugin,
	notr ("{f3b7c9b2-455f-459f-b636-02b2b9a78b7b}"),
	WetterOnlineAnimationPlugin::tr ("Wetter Online (animation)"),
	WetterOnlineAnimationPlugin::tr ("Displays a weather radar animation from wetteronline.de"))

enum State { stateNavigationPage, stateRadarPage, stateRadarImage };

const QString navigationPageUrl (notr ("http://www.wetteronline.de/include/radar_dldl_00_dwddgf.htm"));

WetterOnlineAnimationPlugin::WetterOnlineAnimationPlugin ():
	downloader (new Downloader (this))
{
	downloader->connectSignals (this);
}

WetterOnlineAnimationPlugin::~WetterOnlineAnimationPlugin ()
{
}

void WetterOnlineAnimationPlugin::refresh ()
{
	outputText (tr ("Downloading radar animation (1)..."));
	downloader->startDownload (stateNavigationPage, navigationPageUrl);
}

void WetterOnlineAnimationPlugin::abort ()
{
	downloader->abort ();
}

void WetterOnlineAnimationPlugin::downloadSucceeded (int state, QNetworkReply *reply)
{
	switch ((State)state)
	{
		case stateNavigationPage:
		{
			QString radarPagePath=findInIoDevice (*reply, QRegExp (notr ("a href.*a href=\"\\/([^\"]*)\".*Loop 3 Stunden")), 1);
			if (isBlank (radarPagePath)) OUTPUT_AND_RETURN (tr ("Error: no animation link was found on the navigation page"));
			QString radarPageUrl=qnotr ("http://www.wetteronline.de/%1").arg (radarPagePath);
			downloader->startDownload (stateRadarPage, radarPageUrl);
			outputText (tr ("Downloading radar animation (2)..."));
		} break;
		case stateRadarPage:
		{
			QString radarImagePath=findInIoDevice (*reply, QRegExp (notr ("(daten\\/radar[^\")]*)\"")), 1);
			if (isBlank (radarImagePath)) OUTPUT_AND_RETURN (tr ("Error: no animation was found on the weather page"));
			QString radarImageUrl=qnotr ("http://www.wetteronline.de/%1").arg (radarImagePath);
			downloader->startDownload (stateRadarImage, radarImageUrl);
			outputText (tr ("Downloading radar animation (3)..."));
		} break;
		case stateRadarImage:
		{
			outputText (tr ("Saving radar animation"));
			SkMovie movie (reply);
			if (!movie.getMovie ()->isValid ()) OUTPUT_AND_RETURN (tr ("Error reading the animation"));
			movie.getMovie ()->setSpeed (200);
			outputMovie (movie);
		} break;
	}
}

void WetterOnlineAnimationPlugin::downloadFailed (int state, QNetworkReply *reply, QNetworkReply::NetworkError code)
{
	if (code==QNetworkReply::ContentNotFoundError)
	{
		switch ((State)state)
		{
			case stateNavigationPage:
				outputText (tr ("Error: navigation page not found (404)"));
				break;
			case stateRadarPage:
				outputText (tr ("Error: radar page not found (404)"));
				break;
			case stateRadarImage:
				outputText (tr ("Error: radar animation not found (404)"));
				break;
		}
	}
	else
	{
		outputText (reply->errorString ());
	}
}


void WetterOnlineAnimationPlugin::languageChanged ()
{
	// Nothing to do
}
