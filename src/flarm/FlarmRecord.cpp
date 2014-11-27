#include "FlarmRecord.h"

#include <cassert>
#include <cmath>
#include <iostream>

#include <QtDebug>

#include "src/util/qString.h"
#include "src/i18n/notr.h"
#include "src/nmea/PflaaSentence.h"
#include "src/numeric/Velocity.h"

const int keepaliveTimerInterval=3000;

// ******************
// ** Construction **
// ******************

FlarmRecord::FlarmRecord (QObject *parent, const QString &flarmId): QObject (parent),
	flarmId (flarmId),
	relativeAltitude (0), lastRelativeAltitude (0),
	groundSpeed (0), lastGroundSpeed (0),
	climbRate (0), lastClimbRate (0),
	state (stateUnknown),
	category (Plane::categoryNone)
{
	keepaliveTimer = new QTimer (this);
	landingTimer   = new QTimer (this);
	connect (keepaliveTimer, SIGNAL (timeout ()), this, SLOT (keepaliveTimeout ()));
	connect (landingTimer  , SIGNAL (timeout ()), this, SLOT (landingTimeout   ()));

	keepaliveTimer->start (keepaliveTimerInterval);
}

FlarmRecord::~FlarmRecord ()
{
	// The timers are deleted automatically
}


// ****************
// ** Properties **
// ****************

void FlarmRecord::setRegistration (const QString &registration)
{
	this->registration=registration;
}

void FlarmRecord::setCategory (Plane::Category category)
{
	this->category=category;
}

void FlarmRecord::setFrequency (const QString &frequency)
{
	this->frequency=frequency;
}


// **********
// ** Misc **
// **********

// FIXME sentences may get lost - consider the time of the sentences
bool FlarmRecord::isPlausible () const
{
	// Negative ground speeds are not allowed
	if (groundSpeed < 0) return false;

	// The ground speed cannot be larger than 400 km/h (Mü28: Vne = 380 km/h)
	if (groundSpeed > (400.0 * Velocity::km_h)) return false;

	// FIXME check the position difference

	// The altitude change cannot be larger than 100 m (in one second - 360 km/h
	// vertical velocity)
	if (fabs (relativeAltitude - lastRelativeAltitude) > 100) return false;

	// The ground speed change cannot be larger than 100 km/h (in one second -
	// 2.7 g acceleration)
	if (fabs (groundSpeed - lastGroundSpeed) > (100.0 * Velocity::km_h)) return false;

	return true;
}

void FlarmRecord::processPflaaSentence (const PflaaSentence &sentence)
{
	// We received an update for this plane - defer the timeout
	keepaliveTimer->start (keepaliveTimerInterval);

	// There is no setter for the values because they may only be changed
	// together, lest they become inconsistent.

	// Store the previous values
	// FIXME history should be based on time, not samples - samples may get lost
	lastRelativeAltitude=relativeAltitude;
	lastGroundSpeed     =groundSpeed;
	lastClimbRate       =climbRate;

	// Read the current values from the sentence
	relativeAltitude = sentence.relativeVertical;
	groundSpeed      = sentence.groundSpeed;
	climbRate        = sentence.climbRate;
	relativePosition = QPointF (sentence.relativeEast, sentence.relativeNorth);

	// Add the current position to the position history. We include the current
	// position in the previous positions for some practical reasons:
	//   * for drawing the track, we only have to consider this list, instead of
	//     this list and the current position
	//   * not including the current position would mean adding the previous
	//     position to the list; however, the previous position may be invalid
	//     (at the beginning), so we would have to track that
	// The current position is prepended, so higher indices in the list indicate
	// older positions.
	previousRelativePositions.prepend (relativePosition);

	// Truncate the list to the first
	for (int i=0; i<previousRelativePositions.size ()-20; ++i)
		previousRelativePositions.removeLast ();

	if (isPlausible ())
	{
		// Perform the state transition based on the current values which we
		// just set. This is only done if the current position is plausible,
		// which may not be the case if the transmitting Flarm has lost GPS.
		stateTransition ();
	}
}


// ***********************
// ** Situation methods **
// ***********************

FlarmRecord::FlightSituation FlarmRecord::getSituation () const
{
	//                          ground speed
	//           | 0-2 m/s | 2-10 m/s | 10-40 m/s | >40 m/s
	// altitude  |---------+----------+-----------+--------
	// >40m      |ground   |fly       |fly        |fly
	// 0-40m     |ground   |ground    |low        |fly
	// try to avoid low pass

	if (groundSpeed < 2)
	{
		return groundSituation;
	}
	else if (groundSpeed < 10)
	{
		if (relativeAltitude < 40)
			return groundSituation;
		else
			return flyingSituation;
	}
	else if (groundSpeed < 40)
	{
		if (relativeAltitude < 40)
			return lowSituation;
		else
			return flyingSituation;
	}
	else
	{
		return flyingSituation;
	}
}



// ********************************
// ** FlarmRecord::State methods **
// ********************************

QString FlarmRecord::stateText (flarmState state)
{
	switch (state)
	{
		case FlarmRecord::stateUnknown:   return qApp->translate ("FlarmRecord::State", "Unknown");
		case FlarmRecord::stateOnGround:  return qApp->translate ("FlarmRecord::State", "On ground");
		case FlarmRecord::stateStarting:  return qApp->translate ("FlarmRecord::State", "Departing");
		case FlarmRecord::stateLanding:   return qApp->translate ("FlarmRecord::State", "Approach");
		case FlarmRecord::stateFlying:    return qApp->translate ("FlarmRecord::State", "Flying near airfield");
		case FlarmRecord::stateFlyingFar: return qApp->translate ("FlarmRecord::State", "Out of range");
		// no default
	}

	assert (!notr ("Unhandled state"));
	return notr ("?");
}


// ***********
// ** State **
// ***********

void FlarmRecord::setState (flarmState state, const QString &comment)
{
	(void)comment;
//	flarmState oldState=this->state;
	this->state=state;

//	if (comment.isEmpty ())
//		qDebug () << qnotr ("%1: %2 -> %3")
//			.arg (flarmId)
//			.arg (stateText (oldState))
//			.arg (stateText (state));
//	else
//		qDebug () << qnotr ("%1: %2 -> %3 (%4)")
//			.arg (flarmId)
//			.arg (stateText (oldState))
//			.arg (stateText (state))
//			.arg (comment);
}


void FlarmRecord::keepaliveTimeout ()
{
	qDebug () << "keepAliveTimeout:" << flarmId << "; state =" << getState ();
	keepaliveTimer->stop ();
	switch (getState ())
	{
		case FlarmRecord::stateLanding:
			qDebug () << "landing by timeout1:" << flarmId;
			setState (FlarmRecord::stateOnGround);
			emit landingDetected (flarmId);
			break;
		case FlarmRecord::stateStarting:
			qDebug () << "out of range:" << flarmId;
			break;
		case FlarmRecord::stateFlying:
			qDebug () << "out of range:" << flarmId;
			setState (FlarmRecord::stateFlyingFar);
			break;
		default:
			break;
	}

	// Well, the keepaliver timer timed out, which means that we can't see the
	// plane any more. Remove the record.
	emit remove (flarmId);
}

void FlarmRecord::landingTimeout ()
{
	qDebug () << "landingTimeout:" << flarmId << "; state =" << getState ();
	landingTimer->stop ();
	switch (getState ())
	{
		case FlarmRecord::stateOnGround:
			qDebug () << "landing by timeout2:" << flarmId;
			emit landingDetected (flarmId);
			break;
		default:
			qCritical () << "landingTimeout in invalid state: " << getState () << "; flarmid = " << flarmId;
			break;
	}
}


void FlarmRecord::stateTransition ()
{
	FlightSituation event=getSituation ();

	// [touch and go]: motorglider emit touch and go, others "unexpected touch and go of glider"
	// [landing]: motorglider start landing timer 30s, others emit landing

	// state      | situation       | next state | comments
	// -----------+-----------------+------------+--------------------------
	// unknown    | ground          |on ground   |
	// unknown    | low             |landing     |
	// unknown    | flying          |flying      |
	//
	// on ground  | ground          |-
	// on ground  | low             |starting    |timer active? ("touch and go 2", stop it, [touch and go]) else ("flat start", emit departure)
	// on ground  | flying          |flying      |timer active? ("unexpected touch and go 1", stop it, [touch and go] else ("unexpected start", emit departure)
	//                                            "should not happen. Plane was on ground before, now flying? ignore event?"
	//
	// starting   | ground          |on ground   |"departure aborted", emit landing
	// starting   | low             |-
	// starting   | flying          |flying      |"departure continued"
	//
	// flying     | ground          |on ground   |"unexpected landing from high", [landing]
	// flying     | low             |landing     |"flying low"
	// flying     | flying          |-
	//
	// flying far | ground          |on ground   |"unexpected landing from far", [landing]
	// flying far | low             |landing     |"flying low"
	// flying far | flying          |flying      |"still flying"
	//
	// landing    | ground          |on ground   |"landing continued", [landing]
	// landing    | low             |-
	// landing    | flying          |flying      |"touch and go 3", [touch and go]


	FlarmRecord::flarmState old_state = getState ();

	switch (old_state)
	{
		case stateUnknown:
			switch (event)
			{
				case FlarmRecord::groundSituation: setState (FlarmRecord::stateOnGround); break;
				case FlarmRecord::flyingSituation: setState (FlarmRecord::stateFlying  ); break;
				case FlarmRecord::lowSituation   : setState (FlarmRecord::stateLanding ); break;
				// no default
			}
			break;
		case FlarmRecord::stateOnGround:
			switch (event)
			{
				case FlarmRecord::flyingSituation:
					// should not happen. Plane was on ground before, now flying?
					// ignore event?
					if (landingTimer->isActive ())
					{
						qCritical () << "unexpected touch and go 1: " << flarmId;
						setState (FlarmRecord::stateFlying);
						landingTimer->stop ();
						if (category == Plane::categoryMotorglider)
							emit touchAndGoDetected (flarmId);
						else
							qCritical () << "unexpected touch and go of glider?";
					}
					else
					{
						qCritical () << "unexpected start: " << flarmId;
						setState (FlarmRecord::stateFlying);
						emit departureDetected (flarmId);
					}
					break;
				case FlarmRecord::lowSituation:
					if (landingTimer->isActive ())
					{
						qDebug () << "touch and go 2:" << flarmId;
						setState (FlarmRecord::stateStarting);
						landingTimer->stop ();
						if (category == Plane::categoryMotorglider)
							emit touchAndGoDetected (flarmId);
						else
							qCritical () << "unexpected touchAndGo of glider?";
					}
					else
					{
						setState (FlarmRecord::stateStarting);
						qDebug () << "flat start:" << flarmId;
						emit departureDetected (flarmId);
					}
					break;
				default: break;
			}
			break;
		case FlarmRecord::stateLanding:
			switch (event)
			{
				case FlarmRecord::groundSituation:
					qDebug () << "landing continued:" << flarmId;
					setState (FlarmRecord::stateOnGround);
					if (category == Plane::categoryMotorglider)
						landingTimer->start (30000);
					else
						emit landingDetected (flarmId);
					break;
				case FlarmRecord::flyingSituation:
					qDebug () << "touch and go 3:" << flarmId;
					setState (FlarmRecord::stateFlying);
					if (category == Plane::categoryMotorglider)
						emit touchAndGoDetected (flarmId);
					else
						qCritical () << "unexpected touch and go of glider?";
					break;
				default: break;
			}
			break;
		case FlarmRecord::stateFlying:
			switch (event)
			{
				case FlarmRecord::groundSituation:
					qCritical () << "unexpected landing from high: " << flarmId;
					setState (FlarmRecord::stateOnGround);
					if (category == Plane::categoryMotorglider)
						landingTimer->start (30000);
					else
						emit landingDetected (flarmId);
					break;
				case FlarmRecord::lowSituation: setState (FlarmRecord::stateLanding, "flying low"); break;
				case flyingSituation: break;
				// no default
			}
			break;
		case FlarmRecord::stateFlyingFar:
			switch (event)
			{
				case FlarmRecord::groundSituation:
					qCritical () << "unexpected landing from far: " << flarmId;
					setState (FlarmRecord::stateOnGround);
					if (category == Plane::categoryMotorglider)
						landingTimer->start (30000);
					else
						emit landingDetected (flarmId);
					break;
				case FlarmRecord::lowSituation:
					qDebug () << "flying low:" << flarmId;
					setState (FlarmRecord::stateLanding);
					break;
				case FlarmRecord::flyingSituation:
					qDebug () << "still flying:" << flarmId;
					setState (FlarmRecord::stateFlying);
					break;
				default: break;
			}
			break;
		case FlarmRecord::stateStarting:
			switch (event)
			{
				case FlarmRecord::groundSituation:
					qDebug () << "departure aborted:" << flarmId;
					setState (FlarmRecord::stateOnGround);
					emit landingDetected (flarmId);
					break;
				case FlarmRecord::flyingSituation:
					qDebug () << "departure continued:" << flarmId;
					setState (FlarmRecord::stateFlying);
					break;
				case FlarmRecord::lowSituation: break;
			}
			break;
		// no default
	}
}



