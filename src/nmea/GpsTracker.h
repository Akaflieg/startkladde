#ifndef GPSTRACKER_H_
#define GPSTRACKER_H_

#include <QObject>
#include <QDateTime>

#include "src/numeric/GeoPosition.h"

class QTimer;

class NmeaDecoder;
class GprmcSentence;

/**
 * Tracks GPS state such as the own position
 */
class GpsTracker: public QObject
{
		Q_OBJECT

	public:
		GpsTracker (QObject *parent);
		virtual ~GpsTracker ();

		void setNmeaDecoder (NmeaDecoder *nmeaDecoder);

		void setTimeout (int milliseconds);

		GeoPosition getPosition () const;
		QDateTime getGpsTime () const;

	signals:
		/**
		 * Emitted whenever a new position record is received or no position is
		 * received for a given time.
		 */
		void positionChanged (const GeoPosition &position);
		void gpsTimeChanged (QDateTime gpsTime);

	public slots:
		void gprmcSentence (const GprmcSentence &sentence);

	private:
		NmeaDecoder *nmeaDecoder;

		GeoPosition position;
		QDateTime gpsTime;

		QTimer *timer;

	private slots:
		void timeout ();
};

#endif
