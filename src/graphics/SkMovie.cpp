/*
 * SkMovie.cpp
 *
 *  Created on: 25.07.2010
 *      Author: martin
 */

#include "SkMovie.h"

#include <QByteArray>
#include <QMovie>
#include <QIODevice>

SkMovie::SkMovie ()
{
}

SkMovie::SkMovie (QIODevice *device, const QByteArray &format)
{
	tempFile=QSharedPointer<QTemporaryFile> (new QTemporaryFile ());

	if (tempFile->open ())
	{
		tempFile->write (device->readAll ());
		tempFile->close ();
	}

	movie=QSharedPointer<QMovie> (new QMovie (tempFile->fileName (), format));
}

SkMovie::SkMovie (const QString &fileName, const QByteArray &format)
{
	tempFile=QSharedPointer<QTemporaryFile> (new QTemporaryFile ());

	if (tempFile->open ())
	{
		QFile sourceFile (fileName);
		if (sourceFile.open (QFile::ReadOnly))
		{
			tempFile->write (sourceFile.readAll ());
			sourceFile.close ();
		}
		tempFile->close ();
	}

	movie=QSharedPointer<QMovie> (new QMovie (tempFile->fileName (), format));
}

SkMovie::~SkMovie ()
{
}
