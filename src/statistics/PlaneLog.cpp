#include "PlaneLog.h"

#include <QSet>

#include "src/model/Flight.h"
#include "src/db/cache/Cache.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/text.h"
#include "src/util/qString.h"
#include "src/util/qDate.h"
#include "src/i18n/notr.h"

// ************************
// ** Entry construction **
// ************************

PlaneLog::Entry::Entry ():
	minPassengers (0), maxPassengers (0), numLandings (0), valid (true)
{
}

PlaneLog::Entry::~Entry ()
{
}

// **********************
// ** Entry formatting **
// **********************

QString PlaneLog::Entry::dateText () const
{
	return date.toString (defaultNumericDateFormat ());
}

QString PlaneLog::Entry::numPassengersString () const
{
	if (minPassengers==maxPassengers)
		return QString::number (minPassengers);
	else if (minPassengers==1 && maxPassengers==2)
		return notr ("1/2");
	else
		// Should not happen: entries for non-gliders cannot be merged
		return qnotr ("%1-%2").arg (minPassengers).arg (maxPassengers);
}

QString PlaneLog::Entry::departureTimeText () const
{
	if (departureTime.isValid ())
		return departureTime.toUTC ().toString (tr ("hh:mm 'UTC'"));
	else
		return notr ("-");
}

QString PlaneLog::Entry::landingTimeText () const
{
	if (landingTime.isValid ())
		return landingTime.toUTC ().toString (tr ("hh:mm 'UTC'"));
	else
		return notr ("-");
}

QVariant PlaneLog::Entry::numLandingsText () const
{
	if (numLandings>0)
		return numLandings;
	else
		return notr ("-");
}

QString PlaneLog::Entry::operationTimeText () const
{
	// Operation times >24h may be valid, so use isNull rather than isValid
	if (!operationTime.isNull ())
		return operationTime.toString (tr ("h:mm"));
	else
		return notr ("-");
}

// ********************
// ** Entry creation **
// ********************

/**
 * Create a log entry from a single flight
 */
PlaneLog::Entry PlaneLog::Entry::create (const Flight *flight, Cache &cache)
{
	PlaneLog::Entry entry;

	Plane      *plane     =cache.getNewObject<Plane     > (flight->getPlaneId ());
	Person     *pilot     =cache.getNewObject<Person    > (flight->getPilotId ());

	if (plane) entry.registration=plane->registration.trimmed ();
	if (plane) entry.type=plane->type.trimmed ();

	entry.date=flight->effdatum ();
	if (pilot) entry.pilotName=pilot->formalName ();
	entry.minPassengers=entry.maxPassengers=flight->numPassengers ();
	entry.departureLocation=flight->getDepartureLocation ().trimmed ();
	entry.landingLocation=flight->getLandingLocation ().trimmed ();
	entry.departureTime=flight->hasDepartureTime ()?flight->getDepartureTime ():QDateTime ();
	entry.  landingTime=flight->hasLandingTime   ()?flight->  getLandingTime ():QDateTime ();
	entry.numLandings  =flight->landsHere ()?flight->getNumLandings ():0;
	entry.operationTime=flight->hasDuration ()?flight->flightDuration ():QTime ();
	entry.comments=flight->getComments ().trimmed ();

	entry.valid=flight->finished ();

	delete plane;
	delete pilot;

	return entry;
}

/**
 * Create an entry for a non-empty, sorted list of flights which we know can
 * be merged. All flights must be of the same plane and on the same date.
 */
PlaneLog::Entry PlaneLog::Entry::create (const QList<Flight> &flights, Cache &cache)
{
	assert (!flights.isEmpty ());

	PlaneLog::Entry entry;

	Plane      *plane     =cache.getNewObject<Plane     > (flights.last ().getPlaneId ());
	Person     *pilot     =cache.getNewObject<Person    > (flights.last ().getPilotId ());

	// Values directly determined
	if (plane) entry.registration=plane->registration;
	if (plane) entry.type=plane->type;

    entry.operationTime = QTime(0,0,0,0);
	entry.date=flights.last ().effdatum ();
	if (pilot) entry.pilotName=pilot->formalName ();
	entry.departureLocation=flights.first ().getDepartureLocation ().trimmed ();
	entry.landingLocation=flights.last ().getLandingLocation ().trimmed ();
	entry.departureTime=flights.first ().getDepartureTime ();
	entry.landingTime=flights.last ().getLandingTime ();

	// Values determined from all flights
	entry.minPassengers=entry.maxPassengers=0;
	entry.numLandings=0;
	QStringList comments;
	entry.valid=true;

	int numTowFlights=0;

	foreach (const Flight &flight, flights)
	{
		int numPassengers=flight.numPassengers ();
		if (entry.minPassengers==0 || numPassengers<entry.minPassengers) entry.minPassengers=numPassengers;
		if (entry.maxPassengers==0 || numPassengers>entry.maxPassengers) entry.maxPassengers=numPassengers;

		entry.numLandings+=flight.getNumLandings ();

		if (flight.hasDuration ())
            entry.operationTime=entry.operationTime.addSecs (QTime (0,0,0,0).secsTo (flight.flightDuration ())); // TODO: check flight mode

		if (!isNone (flight.getComments ())) comments << flight.getComments ().trimmed ();
		if (!flight.finished ()) entry.valid=false;

		if (flight.isTowflight ()) ++numTowFlights;
	}

	if (numTowFlights==1)
		comments << tr ("towflight");
	else if (numTowFlights>1)
		comments << tr ("towflights");

	entry.comments=firstToUpper (comments.join (notr ("; ")));

	delete plane;
	delete pilot;

	return entry;
}


// ******************
// ** Construction **
// ******************

PlaneLog::PlaneLog (QObject *parent):
	QAbstractTableModel (parent)
{
}

PlaneLog::~PlaneLog ()
{
}


// **************
// ** Creation **
// **************

/**
 * Makes the log for one plane from a list of flights. The list may contain
 * flights of other planes.
 *
 * @param planeId
 * @param flights
 * @param cache
 * @return
 */
PlaneLog *PlaneLog::createNew (dbId planeId, const QList<Flight> &flights, Cache &cache)
{
	Plane *plane=cache.getNewObject<Plane> (planeId);

	QList<Flight> interestingFlights;

	// Make a list of flights for this plane
	foreach (const Flight &flight, flights)
		if (flight.finished ())
			if (flight.getPlaneId ()==planeId)
				interestingFlights.append (flight);

    std::sort (interestingFlights.begin(), interestingFlights.end());

	// Iterate over all interesting flights, generating logbook entries.
	// Sometimes, we can generate one entry from several flights. These
	// flights are in entryFlights.
	PlaneLog *result=new PlaneLog ();

	QList<Flight> entryFlights;
	const Flight *previousFlight=NULL;
	foreach (const Flight &flight, interestingFlights)
	{
		assert (flight.finished ());

		// We accumulate in entryFlights as long as we can merge flights.
		// Then we create an entry, append it to the list and clear
		// entryFlights.
		if (previousFlight && !flight.collectiveLogEntryPossible (previousFlight, plane))
		{
			// No further merging
			result->entries.append (PlaneLog::Entry::create (entryFlights, cache));
			entryFlights.clear ();
		}

		entryFlights.append (flight);
		previousFlight=&flight;
	}
	result->entries.append (PlaneLog::Entry::create (entryFlights, cache));

	delete plane;

	return result;
}

/**
 * Makes the logs for all pilots that have flights in a given flight list.
 *
 * @param flights
 * @param cache
 * @return
 */
PlaneLog *PlaneLog::createNew (const QList<Flight> &flights, Cache &cache)
{
	// TODO: should we consider tow flights here?

	QSet<dbId> planeIdSet;

	// Determine all planes which have flights
	foreach (const Flight &flight, flights)
		if (flight.finished ())
			planeIdSet.insert (flight.getPlaneId ());

    QList<dbId> planeIds=planeIdSet.values ();
    planeIds.removeAll((quint32) 0);

	// Make a list of the planes and sort it
	QList<Plane> planes;
	foreach (const dbId &id, planeIds)
	{
		try
		{
			planes.append (cache.getObject<Plane> (id));
		}
		catch (...)
		{
			// TODO log error
		}
	}
    std::sort (planes.begin (), planes.end (), Plane::clubAwareLessThan);

	PlaneLog *result=new PlaneLog ();
	foreach (const Plane &plane, planes)
	{
		PlaneLog *planeResult=createNew (plane.getId (), flights, cache);
		result->entries+=planeResult->entries;
		delete planeResult;
	}

	return result;
}


// *********************************
// ** QAbstractTableModel methods **
// *********************************

int PlaneLog::rowCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return entries.size ();
}

int PlaneLog::columnCount (const QModelIndex &index) const
{
	if (index.isValid ())
		return 0;

	return 12;
}

QVariant PlaneLog::data (const QModelIndex &index, int role) const
{
	const Entry &entry=entries[index.row ()];

	// TODO if invalid, add parentheses around name, passengers, 2*location,
	// 3*times, num landings

	if (role==Qt::DisplayRole)
	{
		switch (index.column ())
		{
			case 0: return entry.registration;
			case 1: return entry.type;
			case 2: return entry.dateText ();
			case 3: return entry.pilotName;
			case 4: return entry.numPassengersString ();
			case 5: return entry.departureLocation;
			case 6: return entry.landingLocation;
			case 7: return entry.departureTimeText ();
			case 8: return entry.landingTimeText ();
			case 9: return entry.numLandingsText ();
			case 10: return entry.operationTimeText ();
			case 11: return entry.comments;
			default: assert (false); return QVariant ();
		}
	}
	else
		return QVariant ();
}

QVariant PlaneLog::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role==Qt::DisplayRole)
	{
		if (orientation==Qt::Horizontal)
		{
			switch (section)
			{
				case 0: return tr ("Registration"); break;
				case 1: return tr ("Model"); break;
				case 2: return tr ("Date"); break;
				case 3: return tr ("Pilot"); break;
				case 4: return tr ("Passengers"); break;
				case 5: return tr ("Departure location"); break;
				case 6: return tr ("Landing location"); break;
				case 7: return tr ("Departure time"); break;
				case 8: return tr ("Landing time"); break;
				case 9: return tr ("Number of landings"); break;
				case 10: return tr ("Time airborne"); break;
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
