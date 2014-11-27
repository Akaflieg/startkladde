/*
 * io.cpp
 *
 *  Created on: 17.07.2010
 *      Author: Martin Herrmann
 */

#include "io.h"

#include <QRegExp>
#include <QIODevice>
#include <QString>
#include <QRect>

#include "src/util/qString.h"
#include "src/i18n/notr.h"

QString readLineUtf8 (QIODevice &device)
{
	return QString::fromUtf8 (device.readLine ().constData ());
}

bool findInIoDevice (QIODevice &device, QRegExp &regexp)
{
	while (!device.atEnd ())
	{
		QString line=readLineUtf8 (device);
		if (line.trimmed ().contains (regexp))
    		return true;
	}

	return false;
}

QString findInIoDevice (QIODevice &device, const QRegExp &regexp, int group)
{
	// Make a copy because apparenly we cannot capture in a const QRegExp (but
	// we want to pass a const& so we can use an anonymous value in calls).
	QRegExp re (regexp);

	if (findInIoDevice (device, re))
		return re.cap (group);
	else
		return QString ();
}

std::ostream &operator<< (std::ostream &ostream, const QRect &rect)
{
	ostream << qnotr ("%1x%2+%3+%4").arg (rect.width ()).arg (rect.height ()).arg (rect.x ()).arg (rect.y ());
	return ostream;
}
