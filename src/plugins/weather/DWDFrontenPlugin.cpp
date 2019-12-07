/*
 * WetterOnlineImagePlugin.cpp
 *
 *  Created on: 23.07.2010
 *      Author: martin
 */

#include "DWDFrontenPlugin.h"

#include <QImage>
#include <QRegExp>

#include "src/plugin/factory/PluginFactory.h"
#include "src/net/Downloader.h"
#include "src/util/qString.h"
#include "src/util/io.h"
#include "src/text.h"
#include "src/i18n/notr.h"

REGISTER_PLUGIN (WeatherPlugin, DWDFrontenPlugin)
SK_PLUGIN_DEFINITION (
    DWDFrontenPlugin,
    notr ("{a00e31ec-6d3d-4221-81bd-751a2756937d}"),
    DWDFrontenPlugin::tr ("DWD Surface Chart"),
    DWDFrontenPlugin::tr ("Displays a Bracknell surface chart from dwd.de"))

enum State { stateIndexPage, stateImage };

const QString imageUrl (notr ("https://www.dwd.de/DWD/wetter/wv_spez/hobbymet/wetterkarten/bwk_bodendruck_weu_ana.png"));


DWDFrontenPlugin::DWDFrontenPlugin ():
	downloader (new Downloader (this))
{
	downloader->connectSignals (this);
}

DWDFrontenPlugin::~DWDFrontenPlugin ()
{
}

void DWDFrontenPlugin::refresh ()
{
	outputText (tr ("Download radar image (1)..."));
    downloader->startDownload (0, imageUrl);
}

void DWDFrontenPlugin::abort ()
{
	downloader->abort ();
}

void DWDFrontenPlugin::downloadSucceeded (int state, QNetworkReply *reply)
{
    QByteArray data=reply->readAll ();
    QImage image=QImage::fromData (data);
    if (image.isNull ()) OUTPUT_AND_RETURN (tr ("Error: invalid radar image"));
    outputImage (image);
}

void DWDFrontenPlugin::downloadFailed (int state, QNetworkReply *reply, QNetworkReply::NetworkError code)
{
	if (code==QNetworkReply::ContentNotFoundError)
	{
        outputText (tr ("Error: radar image not found (404)"));
	}
	else
	{
		outputText (reply->errorString ());
	}
}


void DWDFrontenPlugin::languageChanged ()
{
	// Nothing to do
}
