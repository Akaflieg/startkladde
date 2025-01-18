/*
 * ExternalInfoPlugin.cpp
 *
 *  Created on: 22.07.2010
 *      Author: Martin Herrmann
 */

/*
 * TODO:
 *   - parameters as a separate config field (allow spaces!)
 *   - restart behavior: no action, notify, restart (with interval), print message
 *   - working directory: current, program, plugin, other
 *   - allow specifying interpreter
 */

#include "ExternalInfoPlugin.h"

//#include <QDebug>
#include <QSettings>
#include <QFile>
#include <QString>

#include "ExternalInfoPluginSettingsPane.h"
#include "src/plugin/factory/PluginFactory.h"
#include "src/io/SkProcess.h"
#include "src/text.h"
#include "src/util/qString.h"
#include "src/config/Settings.h"
#include "src/i18n/notr.h"

REGISTER_PLUGIN (InfoPlugin, ExternalInfoPlugin)
SK_PLUGIN_DEFINITION (
	ExternalInfoPlugin,
    QUuid::fromString(notr ("{2fbb91be-bde5-4fba-a3c7-69d7caf827a5}")),
	ExternalInfoPlugin::tr ("External"),
	ExternalInfoPlugin::tr ("Receives data from an external program"))

ExternalInfoPlugin::ExternalInfoPlugin (const QString &caption, bool enabled, const QString &command, bool richText):
	InfoPlugin (caption, enabled),
	command (command), richText (richText),
	process (new SkProcess (this))
{
	connect (process, SIGNAL (lineReceived (const QString &)), this, SLOT (lineReceived (const QString &)));
	connect (process, SIGNAL (exited (int, QProcess::ExitStatus)), this, SLOT (processExited (int, QProcess::ExitStatus)));
}

ExternalInfoPlugin::~ExternalInfoPlugin ()
{
	terminate ();
}

PluginSettingsPane *ExternalInfoPlugin::infoPluginCreateSettingsPane (QWidget *parent)
{
	return new ExternalInfoPluginSettingsPane (this, parent);
}

void ExternalInfoPlugin::infoPluginReadSettings (const QSettings &settings)
{
	command =settings.value (notr ("command") , command ).toString ();
	richText=settings.value (notr ("richText"), richText).toBool ();
}

void ExternalInfoPlugin::infoPluginWriteSettings (QSettings &settings)
{
	settings.setValue (notr ("command") , command );
	settings.setValue (notr ("richText"), richText);
}

void ExternalInfoPlugin::start ()
{
	terminate ();

	if (isBlank (command)) OUTPUT_AND_RETURN (tr ("No command specified"));

	QString commandProper;
	QString parameters;
	SkProcess::splitCommand (commandProper, parameters, command);

	QString resolved=resolveFilename (commandProper, Settings::instance ().pluginPaths);
	if (isBlank (resolved)) OUTPUT_AND_RETURN (tr ("Command not found"));
	if (!QFile::exists (resolved)) OUTPUT_AND_RETURN (tr ("Command does not exist"));

	if (!process->startAndWait (resolved+notr (" ")+parameters)) OUTPUT_AND_RETURN (tr ("Error: %1").arg (process->getProcess ()->errorString ()));
	outputText (tr ("Process started"));

	// Note that on Windows, we may have to add the interpreter explicitly.
}

void ExternalInfoPlugin::terminate ()
{
	process->stop ();
}

QString ExternalInfoPlugin::configText () const
{
	if (richText)
		return tr ("%1; rich text").arg (command);
	else
		return tr ("%1; plain text").arg (command);
}

void ExternalInfoPlugin::lineReceived (const QString &line)
{
	outputText (line, richText?Qt::RichText:Qt::PlainText);
}

void ExternalInfoPlugin::processExited (int exitCode, QProcess::ExitStatus exitStatus)
{
	(void)exitCode;
	(void)exitStatus;

	// Restarting (old code)
	// if (warn_on_death) std::cout << "The process for '" << caption << "' died." << std::endl;
	// if (restart_interval>0) QTimer::singleShot (restart_interval*1000, this, SLOT (start ()));
}

void ExternalInfoPlugin::languageChanged ()
{
	// There is no interface for retranslating an external plugin, so we have to
	// restart it
	restart ();
}
