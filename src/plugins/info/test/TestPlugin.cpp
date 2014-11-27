/*
 * TestPlugin.cpp
 *
 *  Created on: 04.07.2010
 *      Author: Martin Herrmann
 */

#include "TestPlugin.h"

#include <QDebug>
#include <QTime>
#include <QSettings>

#include "src/plugin/factory/PluginFactory.h"
#include "TestPluginSettingsPane.h"
#include "src/text.h"
#include "src/util/qString.h"
#include "src/i18n/notr.h"

REGISTER_PLUGIN (InfoPlugin, TestPlugin)
SK_PLUGIN_DEFINITION (
	TestPlugin,
	notr ("{80e116f8-06d5-44ce-802a-e1b727b98af2}"),
	TestPlugin::tr ("Test"),
	TestPlugin::tr ("Outputs a greeting and the current time"))

TestPlugin::TestPlugin (const QString &caption, bool enabled, const QString &greetingName, bool richText):
	InfoPlugin (caption, enabled),
	greetingName (greetingName), richText (richText)
{
	qDebug () << notr ("Creating test plugin");
}

TestPlugin::~TestPlugin ()
{
	qDebug () << notr ("Destroying test plugin");
}

PluginSettingsPane *TestPlugin::infoPluginCreateSettingsPane (QWidget *parent)
{
	return new TestPluginSettingsPane (this, parent);
}

void TestPlugin::infoPluginReadSettings (const QSettings &settings)
{
	greetingName=settings.value (notr ("greetingName"), greetingName).toString ();
	richText    =settings.value (notr ("richText")    , richText    ).toBool   ();
}

void TestPlugin::infoPluginWriteSettings (QSettings &settings)
{
	settings.setValue (notr ("greetingName"), greetingName);
	settings.setValue (notr ("richText")    , richText    );
}

void TestPlugin::start ()
{
	trigger ();
}

void TestPlugin::terminate ()
{

}

void TestPlugin::minuteChanged ()
{
	trigger ();
}

void TestPlugin::trigger ()
{
	// Construct the text parts
	QString helloText=tr ("Hello");

	QString greetingText;
	if (isBlank (greetingName))
		greetingText="";
	else
		greetingText=qnotr (" %1").arg (greetingName);

	QString timeText=tr ("at %1").arg (QTime::currentTime ().toString ());

	// Add color if rich text is set
	if (richText)
	{
		   helloText=qnotr ("<font color=\"#FF3F00\">%1</font>").arg (   helloText);
		greetingText=qnotr ("<font color=\"#3F7F00\">%1</font>").arg (greetingText);
		    timeText=qnotr ("<font color=\"#003FFF\">%1</font>").arg (    timeText);
	}

	// Construct the final text
	QString text=qnotr ("%1%2 %3").arg (helloText, greetingText, timeText);

	// Output the text
	outputText (text, richText?Qt::RichText:Qt::PlainText);
}

QString TestPlugin::configText () const
{
	if (richText)
		return tr ("%1; rich text").arg (greetingName);
	else
		return tr ("%1; plain text").arg (greetingName);
}

void TestPlugin::languageChanged ()
{
	trigger ();
}
