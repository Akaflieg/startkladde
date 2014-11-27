#include "src/time/EventTimeTracker.h"

#include <QDateTime>
#include <QDebug>

#include "src/util/qTime.h"

// TODO we should clean this hash up from time to time (not that it's a big
// deal).

EventTimeTracker::EventTimeTracker ()
{
}

EventTimeTracker::~EventTimeTracker ()
{
}

void EventTimeTracker::eventNow (const QString &value)
{
	_lastEvent.insert (value, QDateTime::currentDateTimeUtc ());
}

QTime EventTimeTracker::timeSinceEvent (const QString &value)
{
	// NB our value is the hash's key
	QDateTime eventTime=_lastEvent.value (value);
	QDateTime currentTime=QDateTime::currentDateTimeUtc ();
	QTime elapsed=fromSeconds (eventTime.secsTo (currentTime));
	return elapsed;
}

bool EventTimeTracker::eventWithin (const QString &value, const QTime &span)
{
	if (!_lastEvent.contains (value))
		return false;

	QDateTime lastTime=_lastEvent[value];
	QDateTime currentTime=QDateTime::currentDateTimeUtc ();
	int secondsSinceLast=lastTime.secsTo (currentTime);
	return secondsSinceLast<=toSeconds (span);
}
