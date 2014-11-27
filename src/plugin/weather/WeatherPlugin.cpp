/*
 * WeatherPlugin.cpp
 *
 *  Created on: 23.07.2010
 *      Author: Martin Herrmann
 */

#include "WeatherPlugin.h"

#include <QTimer>

WeatherPlugin::WeatherPlugin ():
	refreshTimer (new QTimer (this)),
	refreshEnabled (true), refreshInterval (0)
{
	connect (refreshTimer, SIGNAL (timeout ()), this, SLOT (refresh ()));
}

WeatherPlugin::~WeatherPlugin ()
{
}

void WeatherPlugin::outputText (const QString &text, Qt::TextFormat format)
{
	emit textOutput (text, format);
}

void WeatherPlugin::outputImage (const QImage &image)
{
	emit imageOutput (image);
}

void WeatherPlugin::outputMovie (SkMovie &movie)
{
	emit movieOutput (movie);
}

void WeatherPlugin::start ()
{
	refresh ();
	updateRefreshTimer ();
}

void WeatherPlugin::terminate ()
{
	abort ();

	// Don't call disableRefresh - keep the refresh interval
	if (refreshTimer->isActive ())
		refreshTimer->stop ();
}

void WeatherPlugin::enableRefresh (unsigned int seconds)
{
	refreshEnabled=true;
	refreshInterval=seconds;
}

void WeatherPlugin::disableRefresh ()
{
	refreshEnabled=false;
}

void WeatherPlugin::updateRefreshTimer ()
{
	if (refreshEnabled)
	{
		if (refreshInterval>0)
			refreshTimer->start (refreshInterval*1000);
	}
	else
	{
		if (refreshTimer->isActive ())
			refreshTimer->stop ();
	}
}

// ****************
// ** Descriptor **
// ****************

bool WeatherPlugin::Descriptor::nameLessThan (const WeatherPlugin::Descriptor &d1, const WeatherPlugin::Descriptor &d2)
{
	return d1.getName () < d2.getName ();
}

bool WeatherPlugin::Descriptor::nameLessThanP (const WeatherPlugin::Descriptor *d1, const WeatherPlugin::Descriptor *d2)
{
	return nameLessThan (*d1, *d2);
}
