#include "PilotLog.h"

#include <QSet>

#include "src/model/LaunchMethod.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/db/cache/Cache.h"
#include "src/util/qDate.h"


// ************************
// ** Entry construction **
// ************************

PilotLog::Entry::Entry ():
	valid (true)
{
}

PilotLog::Entry::~Entry ()
{
}

// **********************
// ** Entry formatting **
// **********************

QString PilotLog::Entry::dateText () const
{
	return date.toString (defaultNumericDateFormat ());
}

QString PilotLog::Entry::departureTimeText () const
{
	return departureTime.toUTC ().toString (tr ("hh:mm 'UTC'"));
}

QString PilotLog::Entry::landingTimeText () const
{
	return landingTime.toUTC ().toString (tr ("hh:mm 'UTC'"));
}

QString PilotLog::Entry::flightDurationText () const
{
	return flightDuration.toString (tr ("h:mm"));
}


// ********************
// ** Entry creation **
// ********************

/**
 * Makes a log entry from a flight
 *
 * @param flight
 * @param cache
 * @return
 */
PilotLog::Entry PilotLog::Entry::create (const Flight &flight, Cache &cache)
{
	PilotLog::Entry entry;

	Plane        *plane       =cache.getNewObject<Plane       > (flight.getPlaneId        ());
	Person       *pilot       =cache.getNewObject<Person      > (flight.getPilotId        ());
	Person       *copilot     =cache.getNewObject<Person      > (flight.getCopilotId      ());
	LaunchMethod *launchMethod=cache.getNewObject<LaunchMethod> (flight.getLaunchMethodId ());

	entry.date=flight.effdatum ();
	if (plane) entry.planeType=plane->type;
	if (plane) entry.planeRegistration=plane->registration;
	if (pilot) entry.pilot=pilot->formalName ();
	if (copilot) entry.copilot=copilot->formalName ();
	if (launchMethod) entry.launchMethod=launchMethod->logString;
	entry.departureLocation=flight.getDepartureLocation ();
	entry.landingLocation=flight.getLandingLocation ();
	entry.departureTime=flight.getDepartureTime (); // TODO: check flight mode
	entry.landingTime=flight.getLandingTime (); // TODO: check flight mode
	entry.flightDuration=flight.flightDuration (); // TODO: check flight mode
	entry.comments=flight.getComments ();

	entry.valid=flight.finished ();

	delete plane;
	delete pilot;
	delete copilot;
	delete launchMethod;

	return entry;
}


// ******************
// ** Construction **
// ******************

PilotLog::PilotLog (QObject *parent):
	QAbstractTableModel (parent)
{
}

PilotLog::~PilotLog ()
{
}


// **************
// ** Creation **
// **************

/**
 * Makes the log for one pilot from a list of flights. The list may contain
 * flights of other people.
 *
 * @param personId
 * @param flights
 * @param cache
 * @param mode
 * @return
 */
PilotLog *PilotLog::createNew (dbId personId, const QList<Flight> &flights, Cache &cache, FlightInstructorMode mode)
{
	QList<Flight> interestingFlights;

	// Make a list of flights for this person
	foreach (const Flight &flight, flights)
	{
		if (flight.finished ())
		{
			// The person can be the pilot, or (depending on the flight instructor
			// mode) the flight instructor, which is the copilot)
			if (flight.getPilotId ()==personId ||
				(mode==flightInstructorLoose && flight.getCopilotId ()==personId) ||
				(mode==flightInstructorStrict && flight.getType ()==Flight::typeTraining2 && flight.getCopilotId ()==personId))
			{
				interestingFlights.append (flight);
			}
		}
	}

	qSort (interestingFlights);

	// Iterate over all interesting flights, generating logbook entries.
	PilotLog *result=new PilotLog;
	foreach (const Flight &flight, interestingFlights)
		result->entries.append (PilotLog::Entry::create (flight, cache));

	return result;
}

/**
 * Makes the logs for all pilots that have flights in a given flight list.
 *
 * @param flights
 * @param cache
 * @return
 */
PilotLog *PilotLog::createNew (const QList<Flight> &flights, Cache &cache, FlightInstructorMode mode)
{
	QSet<dbId> personIdSet;

	// Determine all people wo have flights
	foreach (const Flight &flight, flights)
	{
		if (flight.finished ())
		{
			personIdSet.insert (flight.getPilotId ());
			personIdSet.insert (flight.getCopilotId ());
		}
	}

	QList<dbId> personIds=personIdSet.toList ();
	personIds.removeAll (0);

	// Make a list of the people and sort it
	QList<Person> people;
	foreach (const dbId &id, personIds)
	{
		try
		{
			people.append (cache.getObject<Person> (id));
		}
		catch (...)
		{
			// TODO log error
		}
	}
	qSort (people);

	PilotLog *result=new PilotLog;
	foreach (const Person &person, people)
	{
		PilotLog *personResult=createNew (person.getId (), flights, cache, mode);
		result->entries+=personResult->entries;
		delete personResult;
	}

	return result;
}


// *********************************
// ** QAbstractTableModel methods **
// *********************************

int PilotLog::rowCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return entries.size ();
}

int PilotLog::columnCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return 12;
}

QVariant PilotLog::data (const QModelIndex &index, int role) const
{
	const Entry &entry=entries[index.row ()];

	// TODO: when invalid, add parentheses around depTime, landTime, flightDur and add comment

	if (role==Qt::DisplayRole)
	{
		switch (index.column ())
		{
			case 0: return entry.dateText ();
			case 1: return entry.planeType;
			case 2: return entry.planeRegistration;
			case 3: return entry.pilot;
			case 4: return entry.copilot;
			case 5: return entry.launchMethod;
			case 6: return entry.departureLocation;
			case 7: return entry.landingLocation;
			case 8: return entry.departureTimeText ();
			case 9: return entry.landingTimeText ();
			case 10: return entry.flightDurationText ();
			case 11: return entry.comments;
			default: assert (false); return QVariant ();
		}
	}
	else
		return QVariant ();
}

QVariant PilotLog::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role==Qt::DisplayRole)
	{
		if (orientation==Qt::Horizontal)
		{
			switch (section)
			{
				case 0: return tr ("Date"); break;
				case 1: return tr ("Model"); break;
				case 2: return tr ("Registration"); break;
				case 3: return tr ("Pilot"); break;
				case 4: return tr ("Copilot"); break;
				case 5: return tr ("Launch method"); break;
				case 6: return tr ("Departure location"); break;
				case 7: return tr ("Landing location"); break;
				case 8: return tr ("Departure time"); break;
				case 9: return tr ("Landing time"); break;
				case 10: return tr ("Flight duration"); break;
				case 11: return tr ("Comments"); break;
			}
		}
		else
		{
			return section+1;
		}
	}

	return QVariant ();
}
