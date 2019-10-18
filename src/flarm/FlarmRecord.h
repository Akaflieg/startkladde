#ifndef FLARM_RECORD_H
#define FLARM_RECORD_H

#include <QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QList>

#include "src/model/Plane.h"

class PflaaSentence;

/**
 * Stores Flarm related data for one plane
 *
 * Note that all numeric data (relative altitude, positions etc.), even data
 * that is received in integer precision from the Flarm, is stored in double
 * precision. This is because most calculations will be performed in double
 * precision and automatic conversion is not possible in all cases. For example,
 * a QVector<QPoint> can not be converted to a QVector<QPointF> automatically.
 * The performance impact of using double instead if integer precision is
 * negligible; for the QVector example, it might even be faster if we can avoid
 * copying the vector.
 *
 * Implementation note: there are no setters for the flight values (position,
 * velocity etc.), not even private ones, because they may only be changed
 * together, lest they become inconsistent.
 */
class FlarmRecord: public QObject
{

		Q_OBJECT

	public:
		// Types
		enum flarmState {stateUnknown, stateOnGround, stateStarting, stateFlying, stateFlyingFar, stateLanding};
		enum FlightSituation {groundSituation, lowSituation, flyingSituation};

		// Construction
		FlarmRecord (QObject* parent, const QString &flarmId);
		virtual ~FlarmRecord ();

		// Properties
		QString getFlarmId          () const { return flarmId; }
		double  getRelativeAltitude () const { return relativeAltitude; }
		double  getGroundSpeed      () const { return groundSpeed; }
		double  getClimbRate        () const { return climbRate; }
		QPointF getRelativePosition () const { return relativePosition; }

		QList<QPointF> getPreviousRelativePositions () const { return previousRelativePositions; }

		flarmState getState () const { return state; }

		QString getRegistration () const { return registration; }

		void setRegistration (const QString &registration);
		void setCategory (Plane::Category category);
		void setFrequency (const QString &frequency);

		// Misc
		bool isPlausible () const;
		void processPflaaSentence (const PflaaSentence &sentence);

		// State methods
		static QString stateText (flarmState state);

	signals:
		// Controller signals (the slots are in FlarmList)
		// While it's not strictly necessary to pass the flarm ID as a parameter
		// here, it simplifies receives which will typically connect to multiple
		// FlarmRecords.
		void departureDetected (const QString &flarmId);
		void landingDetected   (const QString &flarmId);
		void touchAndGoDetected  (const QString &flarmId);
		void remove (const QString &flarmId);

	protected:
		FlightSituation getSituation () const;

	private:
		// Primary data (received from Flarm)
		QString flarmId;
		QPointF relativePosition; // <East, North> in meters
		// previousRelativePositions[0] is the current position, with increasing
		// indices corresponding to older positions. We don't use QQueue with
		// its enqueue and dequeue methods to make this order explicit.
		QList<QPointF> previousRelativePositions;
		double relativeAltitude, lastRelativeAltitude; // In meters
		double groundSpeed     , lastGroundSpeed;      // In meters per second
		double climbRate       , lastClimbRate;        // In meters per second

		// Derived data
		flarmState state;

		// Database data
		QString registration;
		Plane::Category category;
		QString frequency;

		QTimer* keepaliveTimer;
		QTimer* landingTimer;

		void setState (flarmState state, const QString &comment=QString ());
		void stateTransition ();

        // Internal functions for signal emission
        // Filters are applied here
        void _departureDetected(const QString &flarmId);
        void _landingDetected(const QString &flarmId);
        void _touchAndGoDetected(const QString &flarmId);

        // Filters

        // Implements a general filter
        // TRUE if detected departure/landing/touchAndGo shall be registered
        // FALSE otherwise
        bool generalFilterMatch();

	private slots:
		void keepaliveTimeout ();
		void landingTimeout ();

};

#endif
