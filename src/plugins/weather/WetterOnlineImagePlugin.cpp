/*
 * WetterOnlineImagePlugin.cpp
 *
 *  Created on: 23.07.2010
 *      Author: martin
 */

#include "WetterOnlineImagePlugin.h"

#include <QImage>
#include <QRegExp>

#include "src/plugin/factory/PluginFactory.h"
#include "src/net/Downloader.h"
#include "src/util/qString.h"
#include "src/util/io.h"
#include "src/text.h"
#include "src/i18n/notr.h"

REGISTER_PLUGIN (WeatherPlugin, WetterOnlineImagePlugin)
SK_PLUGIN_DEFINITION (
	WetterOnlineImagePlugin,
	notr ("{a00e31ec-6d3d-4221-91bd-751a2756937f}"),
	WetterOnlineImagePlugin::tr ("Wetter Online (image)"),
	WetterOnlineImagePlugin::tr ("Displays a weather image from wetteronline.de"))

enum State { stateIndexPage, stateImage };

const QString indexUrl (notr ("http://www.wetteronline.de/?pcid=pc_radar_map&gid=DL&pid=p_radar_map&sid=Intensity"));

WetterOnlineImagePlugin::WetterOnlineImagePlugin ():
	downloader (new Downloader (this))
{
	downloader->connectSignals (this);
}

WetterOnlineImagePlugin::~WetterOnlineImagePlugin ()
{
}

void WetterOnlineImagePlugin::refresh ()
{
	outputText (tr ("Download radar image (1)..."));
	downloader->startDownload (stateIndexPage, indexUrl);
}

void WetterOnlineImagePlugin::abort ()
{
	downloader->abort ();
}

void WetterOnlineImagePlugin::downloadSucceeded (int state, QNetworkReply *reply)
{
	switch ((State)state)
	{
		case stateIndexPage:
		{
            QString s (reply->readAll());
            QRegExp r ("src=\"(.{1,200})\" class=\"preloaded\"");
            r.indexIn(s);
            QString imagePath= r.cap(1);
            if (isBlank (imagePath)) OUTPUT_AND_RETURN (tr ("Error: no radar image found"));
            QString imageUrl=qnotr ("http://www.wetteronline.de/%1").arg (imagePath);
            downloader->startDownload (stateImage, imageUrl);
			outputText (tr ("Download radar image (2)..."));
		} break;
		case stateImage:
		{
			QByteArray data=reply->readAll ();
			QImage image=QImage::fromData (data);
			if (image.isNull ()) OUTPUT_AND_RETURN (tr ("Error: invalid radar image"));
			outputImage (image);
		} break;
	}
}

void WetterOnlineImagePlugin::downloadFailed (int state, QNetworkReply *reply, QNetworkReply::NetworkError code)
{
	if (code==QNetworkReply::ContentNotFoundError)
	{
		switch ((State)state)
		{
			case stateIndexPage:
				outputText (tr ("Error: page not found (404)"));
				break;
			case stateImage:
				outputText (tr ("Error: radar image not found (404)"));
				break;
		}
	}
	else
	{
		outputText (reply->errorString ());
	}
}


void WetterOnlineImagePlugin::languageChanged ()
{
	// Nothing to do
}
