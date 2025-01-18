/*
 * file.cpp
 *
 *  Created on: 07.07.2010
 *      Author: Martin Herrmann
 */

#include "file.h"

#include <QRegularExpression>
#include <QDebug>

#include "src/util/io.h"


/**
 * Throws FileOpenError
 *
 * @param filename
 * @param regexp
 * @return
 */
std::optional<QRegularExpressionMatch> findInFile (const QString &filename, const QRegularExpression &regexp)
{
	QFile file (filename);
    if (!file.open (QIODevice::ReadOnly)) {
		throw FileOpenError (filename, file.error (), file.errorString ());
    } else {
        return findInIoDevice(file, regexp);
    }
}

/**
 * The passed QRegExp is copied. Use the other findInFile function to access
 * the RegExp.
 *
 * Throws FileOpenError
 *
 * @param filename
 * @param regexp
 * @return
 */
QString findInFileWithCapture (const QString &filename, const QRegularExpression &regexp, int group)
{
	// Make a copy because apparenly we cannot capture in a const QRegExp (but
	// we want to pass a const& so we can use an anonymous value in calls).
    //QRegExp re (regexp);

    std::optional<QRegularExpressionMatch> maybe_match = findInFile(filename, regexp);
    if (maybe_match.has_value()) {
        return maybe_match.value().captured(group);
    } else {
		return QString ();
    }
}
