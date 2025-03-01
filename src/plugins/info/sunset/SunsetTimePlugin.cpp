/*
 * SunsetTimePlugin.cpp
 *
 *  Created on: 07.07.2010
 *      Author: Martin Herrmann
 */

#include "SunsetTimePlugin.h"

#include <QFile>
#include <QSettings>

#include "src/plugin/factory/PluginFactory.h"
#include "src/util/qString.h"
#include "src/util/file.h"
#include "src/util/time.h"
#include "src/text.h"
#include "src/i18n/notr.h"

REGISTER_PLUGIN (InfoPlugin, SunsetTimePlugin)
SK_PLUGIN_DEFINITION (
	SunsetTimePlugin,
    QUuid::fromString(notr ("{1998d604-e819-4aee-af3d-f0c5cee4c508}")),
	SunsetTimePlugin::tr ("Sunset"),
	SunsetTimePlugin::tr ("Displays the sunset time"))

// ******************
// ** Construction **
// ******************

SunsetTimePlugin::SunsetTimePlugin (QString caption, bool enabled, const QString &filename):
	SunsetPluginBase (caption, enabled, filename)
{
}

SunsetTimePlugin::~SunsetTimePlugin ()
{
}


// ********************
// ** Plugin methods **
// ********************

const QString timeFormat=notr ("hh:mm");

/**
 * Calls SunsetPluginBase::start and outputs the sunset time in local time or
 * UTC, depending on the configuration
 */
void SunsetTimePlugin::start ()
{
	SunsetPluginBase::start ();

	QTime sunsetTime=getEffectiveSunset ();
	if (!sunsetTime.isValid ()) return;

	if (displayUtc)
		outputText (sunsetTime.toString (timeFormat)+tr (" UTC", "With leading space"));
	else
		outputText (utcToLocal (sunsetTime).toString (timeFormat));
}

void SunsetTimePlugin::languageChanged ()
{
	restart ();
}

// ************************
// ** InfoPlugin methods **
// ************************

void SunsetTimePlugin::infoPluginReadSettings (const QSettings &settings)
{
	SunsetPluginBase::infoPluginReadSettings (settings);

	displayUtc=settings.value (notr ("displayUtc"), true).toBool ();
}

void SunsetTimePlugin::infoPluginWriteSettings (QSettings &settings)
{
	SunsetPluginBase::infoPluginWriteSettings (settings);

	settings.setValue (notr ("displayUtc"), displayUtc);
}
