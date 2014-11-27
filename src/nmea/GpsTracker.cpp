#include "src/nmea/GpsTracker.h"

#include <QTimer>

#include "src/nmea/NmeaDecoder.h"
#include "src/nmea/GprmcSentence.h"

const int defaultTimeoutMilliseconds=4000; // Update documentation: setTimeout()

/**
 * Creates a GpsTracker instance
 *
 * @param parent the Qt parent object. This object will be deleted automatically
 *               when the parent is destroyed. Can be NULL.
 */
GpsTracker::GpsTracker (QObject *parent): QObject (parent),
	nmeaDecoder (NULL)
{
	timer=new QTimer (this); // Will be deleted by its parent (this)
	timer->setInterval (defaultTimeoutMilliseconds);
	connect (timer, SIGNAL (timeout ()), this, SLOT (timeout ()));
}

GpsTracker::~GpsTracker ()
{
}

/**
 * Sets the NMEA decoder to receive signals from
 */
void GpsTracker::setNmeaDecoder (NmeaDecoder *nmeaDecoder)
{
	if (nmeaDecoder==this->nmeaDecoder)
		return;

	if (this->nmeaDecoder)
	{
		disconnect (this->nmeaDecoder, SIGNAL (gprmcSentence (const GprmcSentence &)), this, SLOT (gprmcSentence (const GprmcSentence &)));
	}

	this->nmeaDecoder=nmeaDecoder;

	if (this->nmeaDecoder)
	{
		connect (this->nmeaDecoder, SIGNAL (gprmcSentence (const GprmcSentence &)), this, SLOT (gprmcSentence (const GprmcSentence &)));
	}
}

/**
 * Sets the timeout (in milliseconds)
 *
 * After the timeout, the own position will be set to invalid. The default
 * timeout is 4000 milliseconds.
 */
void GpsTracker::setTimeout (int milliseconds)
{
	timer->setInterval (milliseconds);
}

/**
 * Get the most recent position received
 */
GeoPosition GpsTracker::getPosition () const
{
	return position;
}

/**
 * Gets the most recent GPS time received
 * @return
 */
QDateTime GpsTracker::getGpsTime () const
{
	return gpsTime;
}

/**
 * Updates the state based on a GPRMC sentence
 */
void GpsTracker::gprmcSentence (const GprmcSentence &sentence)
{
	if (!sentence.isValid ()) return;

	timer->start ();

	bool positionHasChanged=(sentence.position!=this->position);

	this->gpsTime=sentence.timestamp;
	this->position=sentence.position;

	if (positionHasChanged)
		emit positionChanged (this->position);

	// The GPS time typically changes with every GPRMC sentence
	emit gpsTimeChanged (this->gpsTime);
}

void GpsTracker::timeout ()
{
	this->position=GeoPosition ();
	this->gpsTime =QDateTime ();

	emit positionChanged (this->position);
	emit gpsTimeChanged  (this->gpsTime );
}
