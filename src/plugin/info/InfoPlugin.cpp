/*
 * InfoPlugin.cpp
 *
 *  Created on: 03.07.2010
 *      Author: Martin Herrmann
 */

#include "InfoPlugin.h"

#include <QSettings>
#include <QDebug>

#include "src/plugin/info/InfoPluginSettingsPane.h"
#include "src/i18n/notr.h"

InfoPlugin::InfoPlugin (QString caption, bool enabled):
	caption (caption), enabled (enabled)
{
}

InfoPlugin::~InfoPlugin ()
{
}

void InfoPlugin::outputText (const QString &text, Qt::TextFormat format)
{
	emit textOutput (text, format);
}

/**
 * Returns an InfoPluginSettingsPane with fields for the caption and the pane
 * created by infoPluginCreateSettingsPane
 *
 * The caller takes ownership of the created PluginSettingsPane.
 *
 * @param parent the parent widget
 * @return a newly created PluginSettingsPane
 */
PluginSettingsPane *InfoPlugin::createSettingsPane (QWidget *parent)
{
	return new InfoPluginSettingsPane (this, parent);
}

/**
 * Reads the common settings for this info plugin and calls
 * infoPluginReadSettings to read the plugin specific settings
 *
 * @param settings the QSettings to read the settings from
 * @see Plugin::readSettings
 */
void InfoPlugin::readSettings (const QSettings &settings)
{
	caption=settings.value (notr ("caption"), getName ()).toString ();
	enabled=settings.value (notr ("enabled"), true).toBool ();
	infoPluginReadSettings (settings);
}

/**
 * Writes the common settings for this info plugin and calls
 * infoPluginWriteSettings to write the plugin specific settings
 *
 * @param settings the QSettings to write the settings to
 * @see Plugin::writeSettings
 */
void InfoPlugin::writeSettings (QSettings &settings)
{
	settings.setValue (notr ("caption"), caption);
	settings.setValue (notr ("enabled"), enabled);
	infoPluginWriteSettings (settings);
}


// ****************
// ** Descriptor **
// ****************

bool InfoPlugin::Descriptor::nameLessThan (const InfoPlugin::Descriptor &d1, const InfoPlugin::Descriptor &d2)
{
	return d1.getName () < d2.getName ();
}

bool InfoPlugin::Descriptor::nameLessThanP (const InfoPlugin::Descriptor *d1, const InfoPlugin::Descriptor *d2)
{
	return nameLessThan (*d1, *d2);
}
