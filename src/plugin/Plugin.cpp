/*
 * Plugin.cpp
 *
 *  Created on: 03.07.2010
 *      Author: Martin Herrmann
 */

#include "Plugin.h"

#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QFileDialog>

#include "src/text.h"
#include "src/util/qString.h"
#include "src/util/environment.h"
#include "src/i18n/notr.h"
#include "src/i18n/LanguageChangeNotifier.h"

Plugin::Plugin ()
{
	LanguageChangeNotifier::subscribe (this);
}

Plugin::~Plugin ()
{
}

void Plugin::restart ()
{
	terminate ();
	start ();
}

/**
 * Determines whether a file name is absolute
 *
 * A file name is considered absolute if QFileInfo::isAbsolute returns true, or
 * if the file name starts with "./" (or ".\\").
 *
 * @param filename a file name
 * @return true if filename is absolute, false if not
 */
bool Plugin::filenameIsAbsolute (const QString &filename)
{
	// We treat file names with the explicit directory ./ as absolute
	if (filename.startsWith (notr ("./"))) return true;
	if (filename.startsWith (notr (".\\"))) return true;
	if (QFileInfo (filename).isAbsolute ()) return true;

	return false;
}

/**
 *
 * @param filename
 * @param pluginPaths usually, use Settings::instance ().pluginPaths
 * @return
 */
QString Plugin::resolveFilename (const QString &filename, const QStringList &pluginPaths)
{
	if (isBlank (filename))
		return "";

	// Absolute file names are not changed
	if (filenameIsAbsolute (filename))
		return filename;

	// Search in the plugin paths
	foreach (const QString &path, pluginPaths)
	{
		// TODO: Qt pathname construction?
		QString full=path+notr ("/")+filename;
		if (QFile::exists (full))
			return full;
	}

	// Search in the current directory
	if (QFile::exists (filename))
		return filename;

	// Search in the system path
	QStringList systemPath=getSystemPath ();
	foreach (const QString &path, systemPath)
	{
		// TODO: Qt pathname construction?
		QString full=path+notr ("/")+filename;
		if (QFile::exists (full))
			return full;
	}


	// Not found
	return QString ();
}

QString Plugin::browse (const QString &currentFile, const QString &filter, const QStringList &pluginPaths, QWidget *parent)
{
	QString resolved=Plugin::resolveFilename (currentFile, pluginPaths);

	QString dir;
	if (resolved.isEmpty ())
		dir=notr (".");
	else
		dir=QFileInfo (resolved).dir ().path ();

	return QFileDialog::getOpenFileName (
		parent,
		tr ("Select file"),
		dir,
		filter,
		NULL,
        {}
		);
}
