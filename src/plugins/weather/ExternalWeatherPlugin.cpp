/*
 * ExternalWeatherPlugin.cpp
 *
 *  Created on: 22.07.2010
 *      Author: Martin Herrmann
 */

#include "ExternalWeatherPlugin.h"

//#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QImage>
#include <QMovie>

#include "src/plugin/factory/PluginFactory.h"
#include "src/io/SkProcess.h"
#include "src/text.h"
#include "src/util/qString.h"
#include "src/config/Settings.h"
#include "src/graphics/SkMovie.h"
#include "src/i18n/notr.h"

REGISTER_PLUGIN (WeatherPlugin, ExternalWeatherPlugin)
SK_PLUGIN_DEFINITION (
	ExternalWeatherPlugin,
    QUuid::fromString(notr ("{01db73ff-1473-4aeb-b297-13398927005c}")),
	ExternalWeatherPlugin::tr ("External"),
	ExternalWeatherPlugin::tr ("External weather plugin"))

ExternalWeatherPlugin::ExternalWeatherPlugin (const QString &command):
	command (command),
	process (new SkProcess (this))
{
	connect (process, SIGNAL (lineReceived (const QString &)), this, SLOT (lineReceived (const QString &)));
	connect (process, SIGNAL (exited (int, QProcess::ExitStatus)), this, SLOT (processExited (int, QProcess::ExitStatus)));
}

ExternalWeatherPlugin::~ExternalWeatherPlugin ()
{
	terminate ();
}

void ExternalWeatherPlugin::refresh ()
{
	outputText (tr ("Starting process..."));

	if (isBlank (command)) OUTPUT_AND_RETURN (tr ("No command specified"));

	QString commandProper;
	QString parameters;
	SkProcess::splitCommand (commandProper, parameters, command);

	QString resolved=resolveFilename (commandProper, Settings::instance ().pluginPaths);
	if (isBlank (resolved)) OUTPUT_AND_RETURN (tr ("Command not found"));
	if (!QFile::exists (resolved)) OUTPUT_AND_RETURN (tr ("Command does not exist"));

	if (!process->startAndWait (resolved+notr (" ")+parameters)) OUTPUT_AND_RETURN (tr ("Error: %1").arg (process->getProcess ()->errorString ()));
	outputText (tr ("Process started"));
}

void ExternalWeatherPlugin::abort ()
{
	process->stop ();
}


void ExternalWeatherPlugin::lineReceived (const QString &line)
{
	if (line.startsWith (notr ("[MSG]"), Qt::CaseInsensitive))
	{
        QRegularExpression rx (notr ("\\[MSG\\]\\s*\\[(.*)\\]") );
        auto rxm = rx.match(line);
        if (rxm.hasMatch()) {
            QString text=rxm.captured(1);
			// An even number of backslashes, followed by a backslash and an n ==> newline
			// (\\)*\n ==> newline
			// Regexp escaping: (\\\\)*\\n ==> newline
			// C escaping: (\\\\\\\\)*\\\\n ==> \n
            outputText (text.replace (QRegularExpression (notr ("(\\\\\\\\)*\\\\n")), notr ("\n")).replace (notr ("\\\\"), notr ("\\")));
		}
	}
	else if (line.startsWith (notr ("[IMG]"), Qt::CaseInsensitive))
	{
        QRegularExpression rx (notr ("\\[IMG\\]\\s*\\[(.*)\\]") );
        auto rxm = rx.match(line);
        if (rxm.hasMatch()) {
            QString filename=rxm.captured(1);
			QImage image (filename);

			if (image.isNull ())
				outputText (tr ("Cannot load image:\n%1").arg (filename));
			else
				outputImage (image);
		}
	}
	else if (line.startsWith (notr ("[MOV]"), Qt::CaseInsensitive))
	{
        QRegularExpression rx (notr ("\\[MOV\\]\\s*\\[(.*)\\]") );
        auto rxm = rx.match(line);
        if (rxm.hasMatch()) {
            QString filename=rxm.captured(1);
			SkMovie movie (filename);
			if (movie.getMovie ()->isValid ())
				outputMovie (movie);
			else
				outputText (tr ("Cannot load animation:\n%1").arg (filename));
		}
	}
}

void ExternalWeatherPlugin::processExited (int exitCode, QProcess::ExitStatus exitStatus)
{
	(void)exitCode;
	(void)exitStatus;

	// That's cool
}

void ExternalWeatherPlugin::languageChanged ()
{
	// There is no interface for retranslating an external plugin, so we have to
	// restart it
	restart ();
}
