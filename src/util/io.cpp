/*
 * io.cpp
 *
 *  Created on: 17.07.2010
 *      Author: Martin Herrmann
 */

#include "io.h"

#include <QRegularExpression>
#include <QIODevice>
#include <QString>
#include <QRect>
#include <optional>

#include "src/util/qString.h"
#include "src/i18n/notr.h"

QString readLineUtf8 (QIODevice &device)
{
	return QString::fromUtf8 (device.readLine ().constData ());
}

 std::optional<QRegularExpressionMatch> findInIoDevice (QIODevice &device, const QRegularExpression &regexp)
{
	while (!device.atEnd ())
	{
		QString line=readLineUtf8 (device);
        QRegularExpressionMatch m = regexp.match(line.trimmed());
        if (m.hasMatch())
            return m;
	}

    return {};
}

QString findInIoDeviceWithCapture (QIODevice &device, const QRegularExpression &regexp, int group)
{
    std::optional<QRegularExpressionMatch> maybe_match = findInIoDevice(device, regexp);
    if (maybe_match.has_value()) {
        return maybe_match.value().captured(group);
    } else {
        return QString();
    }
}

std::ostream &operator<< (std::ostream &ostream, const QRect &rect)
{
	ostream << qnotr ("%1x%2+%3+%4").arg (rect.width ()).arg (rect.height ()).arg (rect.x ()).arg (rect.y ());
	return ostream;
}
