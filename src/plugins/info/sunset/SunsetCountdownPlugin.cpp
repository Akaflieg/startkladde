/*
 * SunsetCountdownPlugin.cpp
 *
 *  Created on: 07.07.2010
 *      Author: Martin Herrmann
 */

#include "SunsetCountdownPlugin.h"

#include <cstdlib>

#include <QFile>

#include "src/plugin/factory/PluginFactory.h"
#include "src/util/qString.h"
#include "src/util/file.h"
#include "src/util/time.h"
#include "src/text.h"
#include "src/i18n/notr.h"

REGISTER_PLUGIN (InfoPlugin, SunsetCountdownPlugin)
SK_PLUGIN_DEFINITION (
	SunsetCountdownPlugin,
    QUuid::fromString(notr ("{9735dfd7-ddfd-476c-affd-edbf196e0279}")),
	SunsetCountdownPlugin::tr ("Sunset countdown"),
	SunsetCountdownPlugin::tr ("Displays the time remaining until sunset"))

// ******************
// ** Construction **
// ******************

SunsetCountdownPlugin::SunsetCountdownPlugin (QString caption, bool enabled, const QString &filename):
	SunsetPluginBase (caption, enabled, filename)
{
}

SunsetCountdownPlugin::~SunsetCountdownPlugin ()
{
}


// ********************
// ** Plugin methods **
// ********************

/**
 * Calls SunsetPluginBase::start and update
 */
void SunsetCountdownPlugin::start ()
{
	SunsetPluginBase::start ();
	update ();
}

void SunsetCountdownPlugin::minuteChanged ()
{
	update ();
}

void SunsetCountdownPlugin::languageChanged ()
{
	update ();
}


// *******************
// ** Functionality **
// *******************

/**
 * Outputs the time until sunset, in red color if negative
 */
void SunsetCountdownPlugin::update ()
{
	QTime sunsetTime=nullSeconds (getEffectiveSunset ());
	if (!sunsetTime.isValid ()) return;

	QTime currentTime=nullSeconds (currentTimeUtc ());

	int seconds=currentTime.secsTo (sunsetTime);

	QString duration=formatDuration (seconds, false);

	QString output;
	if (seconds<0)
		output=qnotr ("<font color=\"#FF0000\">%2</font>").arg (duration);
	else
		output=duration;

	outputText (output, Qt::RichText);
}
