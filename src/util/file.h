/*
 * file.h
 *
 *  Created on: 07.07.2010
 *      Author: Martin Herrmann
 */

#ifndef FILE_H_
#define FILE_H_

#include <QFile> // Required for QFile::FileError

class FileOpenError
{
	public:
		FileOpenError (const QString &filename, QFile::FileError error, const QString &errorString):
			filename (filename), error (error), errorString (errorString) {}

		QString filename;
		QFile::FileError error;
		QString errorString;
};

std::optional<QRegularExpressionMatch> findInFile (const QString &filename, QRegularExpression &regexp);
QString findInFileWithCapture (const QString &filename, const QRegularExpression &regexp, int group);


#endif
