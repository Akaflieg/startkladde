/*
 * Implementation notes:
 *   - All accesses to internal data, even single values, must be protected by
 *     dataMutex. Use:
 *     - synchronizedReturn (dataMutex, value) if just a value is returned
 *     - synchronized (dataMutex) { ... } if this doesn't cause warnings about
 *       control reaching the end of the function (in methods which return a
 *       value)
 *     - QMutexLocker (&dataMutex) otherwise
 *     - dataMutex.lock () only when there's good reason (and document the
 *       reason)
 *
 * Improvements:
 *   - log an error if an invalid ID is passed to the get by ID functions
 *   - case insensitive sorting for string lists
 *   - case insensitive uniqueness for string lists
 *
 * Here's the call schema for an added object:
 *   DbEvent -> handleDbEvent<T> -.-> ObjectAdded<T> -.----> updateHashesObjectAdded<Person>
 *                                |                   |----> updateHashesObjectAdded<Plane>
 *                                |                   '----> updateHashesObjectAdded<LaunchMethod>
 *                                '-> ObjectAdded<Flight> -> updateHashesObjectAdded<Flight>
 * The branches are cause by (partial) template specialization. For an updated
 * and a deleted object, the schema is similar.
 */

#include "Cache.h"

#include <iostream>

#include <QSet>
#include <QMap>

#include "src/model/Flight.h"
#include "src/model/LaunchMethod.h"
#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/flarm/flarmNet/FlarmNetRecord.h"
#include "src/db/Database.h"
#include "src/concurrent/synchronized.h"
#include "src/util/qString.h"
#include "src/container/SortedSet_impl.h"
#include "src/db/Query.h"

// ******************
// ** Construction **
// ******************

Cache::Cache (Database &db):
	// db (db), dataMutex (QMutex::Recursive)
	db (db), dataMutex ()
{
	connect (&db, SIGNAL (dbEvent (DbEvent)), this, SLOT (dbChanged (DbEvent)));
}

Cache::~Cache ()
{
}


// ****************
// ** Properties **
// ****************

Database &Cache::getDatabase ()
{
	return db;
}


// *************************
// ** Data access helpers **
// *************************

// These template methods return a reference to concrete data for a
// given type. They must be specialized before they are used.

// Specialize list getters (const)
template<> const EntityList<Plane         > &Cache::objectList<Plane         > () const { return planes         ; }
template<> const EntityList<Person        > &Cache::objectList<Person        > () const { return people         ; }
template<> const EntityList<LaunchMethod  > &Cache::objectList<LaunchMethod  > () const { return launchMethods  ; }
template<> const EntityList<FlarmNetRecord> &Cache::objectList<FlarmNetRecord> () const { return flarmNetRecords; }

// Specialize list getters (non-const)
template<> EntityList<Plane         > &Cache::objectList<Plane         > () { return planes         ; }
template<> EntityList<Person        > &Cache::objectList<Person        > () { return people         ; }
template<> EntityList<LaunchMethod  > &Cache::objectList<LaunchMethod  > () { return launchMethods  ; }
template<> EntityList<FlarmNetRecord> &Cache::objectList<FlarmNetRecord> () { return flarmNetRecords; }

// Specialize hash getters (const)
template<> const QHash<dbId, Plane         > &Cache::objectsByIdHash<Plane         > () const { return          planesById; }
template<> const QHash<dbId, Person        > &Cache::objectsByIdHash<Person        > () const { return          peopleById; }
template<> const QHash<dbId, Flight        > &Cache::objectsByIdHash<Flight        > () const { return         flightsById; }
template<> const QHash<dbId, LaunchMethod  > &Cache::objectsByIdHash<LaunchMethod  > () const { return   launchMethodsById; }
template<> const QHash<dbId, FlarmNetRecord> &Cache::objectsByIdHash<FlarmNetRecord> () const { return flarmNetRecordsById; }

// Specialize hash getters (non-const)
template<> QHash<dbId, Plane         > &Cache::objectsByIdHash<Plane         > () { return          planesById; }
template<> QHash<dbId, Person        > &Cache::objectsByIdHash<Person        > () { return          peopleById; }
template<> QHash<dbId, Flight        > &Cache::objectsByIdHash<Flight        > () { return         flightsById; }
template<> QHash<dbId, LaunchMethod  > &Cache::objectsByIdHash<LaunchMethod  > () { return   launchMethodsById; }
template<> QHash<dbId, FlarmNetRecord> &Cache::objectsByIdHash<FlarmNetRecord> () { return flarmNetRecordsById; }


// ************************
// ** Generic refreshing **
// ************************


void Cache::refreshStatic(OperationMonitorInterface monitor)
{
    monitor.status(tr("Retrieving flights per plane"));
    synchronized(dataMutex)
    {
        roughNumberOfFlightsByPlaneIdLastYear = db.flightsPerPlane(QDate::currentDate().addYears(-1), QDate::currentDate());
        roughNumberOfFlightsByPersonIdLastYear = db.flightsPerPerson(QDate::currentDate().addYears(-1), QDate::currentDate());

        numberOfFlightsByPersonIdLastHalfYear = db.flightsPerPersonAsPic(QDate::currentDate().addDays(-183), QDate::currentDate().addDays(-1));
        numberOfHoursByPersonIdLastHalfYear = db.hoursPerPersonAsPic(QDate::currentDate().addDays(-183), QDate::currentDate().addDays(-1));
    }
}

template<class T> void Cache::refreshObjects (OperationMonitorInterface monitor)
{
	monitor.status (tr ("Retrieving %1").arg (T::objectTypeDescriptionPlural ()));

	// Get the list from the database
	QList<T> newObjects=db.getObjects<T> ();

	synchronized (dataMutex)
	{
		// Store the object list
		objectList<T> ()=newObjects;

		// Update hashes
		QHash<dbId, T> &byIdHash=objectsByIdHash<T> ();
		byIdHash.clear ();
		clearHashes<T> ();
		foreach (const T &object, newObjects)
		{
			byIdHash.insert (object.getId (), object);
			updateHashesObjectAdded<T> (object);
		}
	}
}

// *************************
// ** Specific refreshing **
// *************************

void Cache::refreshPlanes (OperationMonitorInterface monitor)
{
	refreshObjects<Plane> (monitor);

    synchronized(dataMutex)
    {
        QMultiMap<int, Plane> sortMap;
        for (int i = 0; i < planes.size(); i++)
        {
            sortMap.insert(
                (-1)*roughNumberOfFlightsByPlaneIdLastYear.value(planes.at(i).getId(), 0),
                planes.at(i)
            );
        }

        planesSortedByUsage = sortMap.values();
    }
}

void Cache::refreshPeople (OperationMonitorInterface monitor)
{
	refreshObjects<Person> (monitor);

    synchronized(dataMutex)
    {
        QMultiMap<int, Person> sortMap;
        for (int i = 0; i < people.size(); i++)
        {
            sortMap.insert(
                (-1)*roughNumberOfFlightsByPersonIdLastYear.value(people.at(i).getId(), 0),
                people.at(i)
            );
        }

        peopleSortedByFrequency = sortMap.values();
    }
}

void Cache::refreshLaunchMethods (OperationMonitorInterface monitor)
{
	refreshObjects<LaunchMethod> (monitor);
}

void Cache::refreshFlarmNetRecords (OperationMonitorInterface monitor)
{
	refreshObjects<FlarmNetRecord> (monitor);
}

void Cache::refreshFlightsOf (const QString &description, const QDate &date,
	EntityList<Flight> &targetList, QDate *targetDate, OperationMonitorInterface monitor)
{
	monitor.status (tr ("Retrieving %1").arg (description));

	// Get the list from the database
	QList<Flight> newFlights;
	if (date.isNull ())
		newFlights=db.getPreparedFlights ();
	else
		newFlights=db.getFlightsDate (date);

	synchronized (dataMutex)
	{
		// Remove the old flights from the hashes
		foreach (const Flight &flight, targetList.getList ())
		{
			flightsById.remove (flight.getId ());
			updateHashesObjectDeleted<Flight> (flight.getId (), &flight);
		}

		// Store the date and the new flights
		if (targetDate) (*targetDate)=date;
		targetList.replaceList (newFlights);

		// Add the new flights to the hashes
		foreach (const Flight &flight, newFlights)
		{
			flightsById.insert (flight.getId (), flight);
			updateHashesObjectAdded<Flight> (flight);
		}
	}
}

void Cache::refreshFlightsToday (OperationMonitorInterface monitor)
{
	refreshFlightsOf (tr ("flights of today"),
		QDate::currentDate (), flightsToday, &todayDate, monitor);
}

void Cache::refreshFlightsOther (OperationMonitorInterface monitor)
{
	if (otherDate.isNull ()) return;
	QLocale locale;
	refreshFlightsOf (tr ("flights of %1").arg (locale.toString (otherDate, QLocale::ShortFormat)),
		otherDate, flightsOther, NULL, monitor);
}

void Cache::fetchFlightsOther (QDate date, OperationMonitorInterface monitor)
{
	if (date.isNull ()) return;
	QLocale locale;
	refreshFlightsOf (tr ("flights of %1").arg (locale.toString (date, QLocale::ShortFormat)),
		date, flightsOther, &otherDate, monitor);
}

void Cache::refreshPreparedFlights (OperationMonitorInterface monitor)
{
	refreshFlightsOf (tr ("prepared flights"),
		QDate (), preparedFlights, NULL, monitor);
}

void Cache::refreshLocations (OperationMonitorInterface monitor)
{
	monitor.status (tr ("locations"));

	QStringList newLocations=db.listLocations ();
	synchronized (dataMutex) locations=newLocations;
}

void Cache::refreshAccountingNotes (OperationMonitorInterface monitor)
{
	monitor.status (tr ("accounting notes"));

	QStringList newAccountingNotes=db.listAccountingNotes ();
	synchronized (dataMutex) accountingNotes=newAccountingNotes;
}

void Cache::refreshAll (OperationMonitorInterface monitor)
{
	// The other hashes will be cleared by the refresh methods
	clearMultiTypeHashes ();
	clearHashes<Flight> ();

	// Refresh planes and people before refreshing flights!
    monitor.progress (0,  9); refreshStatic          (monitor);
    monitor.progress (1,  9); refreshPlanes          (monitor);
    monitor.progress (2,  9); refreshPeople          (monitor);
    monitor.progress (3,  9); refreshLaunchMethods   (monitor);
    monitor.progress (4,  9); refreshFlightsToday    (monitor);
    monitor.progress (5,  9); refreshFlightsOther    (monitor);
    monitor.progress (6,  9); refreshPreparedFlights (monitor);
    monitor.progress (7,  9); refreshLocations       (monitor);
    monitor.progress (8,  9); refreshAccountingNotes (monitor);
    monitor.progress (9,  9); refreshFlarmNetRecords (monitor);
    monitor.progress (10, 9, tr ("Finished"));
}

void Cache::refreshFlights (OperationMonitorInterface monitor)
{
	clearHashes<Flight> ();

	// Refresh planes and people before refreshing flights!
	monitor.progress (0, 3); refreshFlightsToday    (monitor);
	monitor.progress (1, 3); refreshFlightsOther    (monitor);
	monitor.progress (2, 3); refreshPreparedFlights (monitor);
	monitor.progress (3, 3, tr ("Finished"));
}



// *********************
// ** Change handling **
// *********************

// This template is specialized for T==Flight
template<class T> void Cache::objectAdded (const T &object)
{
	// Add the object to the cache
	synchronized (dataMutex)
	{
		// Object list
		objectList<T> ().append (object);

		// By-ID and specific hashes
		objectsByIdHash<T> ().insert (object.getId (), object);
		updateHashesObjectAdded<T> (object);
	}
}

template<> void Cache::objectAdded<Flight> (const Flight &flight)
{
	synchronized (dataMutex)
	{
		bool interested=true;

		if (flight.isPrepared ())
			preparedFlights.append (flight);
		else if (flight.effdatum ()==todayDate)
			flightsToday.append (flight);
		else if (flight.effdatum ()==otherDate)
			// If otherDate is the same as today, this is not reached.
			flightsOther.append (flight);
		else
			// We're not interested in this flight
			interested=false;

		if (interested)
		{
			// By-ID and specific hashes
			objectsByIdHash<Flight> ().insert (flight.getId (), flight);
			updateHashesObjectAdded<Flight> (flight);
		}
	}
}

// This template is specialized for T==Flight
template<class T> void Cache::objectDeleted (dbId id)
{
	// Remove the object from the cache
	synchronized (dataMutex)
	{
		const T *old=getNewObject<T> (id);

		// Object list
		objectList<T> ().removeById (id);

		// By-ID and specific hashes
		objectsByIdHash<T> ().remove (id);
		updateHashesObjectDeleted<T> (id, old);
	}
}

template<> void Cache::objectDeleted<Flight> (dbId id)
{
	synchronized (dataMutex)
	{
		const Flight *old=getNewObject<Flight> (id);

		// If any of the lists contain this flight, remove it
		preparedFlights.removeById (id);
		flightsToday.removeById (id);
		flightsOther.removeById (id);

		// By-ID and specific hashes
		objectsByIdHash<Flight> ().remove (id);
		updateHashesObjectDeleted<Flight> (id, old);
	}
}

// This template is specialized for T==Flight
template<class T> void Cache::objectUpdated (const T &object)
{
	// TODO if the object is not in the cache, add it and log an error

	// Update the cache
	synchronized (dataMutex)
	{
		const T *old=getNewObject<T> (object.getId ());

		// Object list
		objectList<T> ().replaceById (object.getId (), object);

		// By-ID and specific hashes
		objectsByIdHash<T> ().insert (object.getId (), object);
		updateHashesObjectUpdated<T> (object, old);
	}
}

template<> void Cache::objectUpdated<Flight> (const Flight &flight)
{
	// If the date or the prepared status of a flight changed, we may have to
	// relocate it to a different list. If the date is changed, it may not be
	// on any list at all any more; or it may not have been on any list before
	// (although the UI does not provide a way to modify a flight that is not
	// on one of these lists, but something like that may well be added, and
	// even if not, we'd still have to handle this case).

	// Determine which list the flight should be in (or none). Replace it if
	// it already exists, add it if not, and remove it from the other lists.

	synchronized (dataMutex)
	{
		bool interested=true;

		const Flight *old=getNewObject<Flight> (flight.getId ());

		if (flight.isPrepared ())
		{
			preparedFlights.replaceOrAdd (flight.getId (), flight);
			flightsToday.removeById (flight.getId ());
			flightsOther.removeById (flight.getId ());
		}
		else if (flight.effdatum ()==todayDate)
		{
			preparedFlights.removeById (flight.getId ());
			flightsToday.replaceOrAdd (flight.getId (), flight);
			flightsOther.removeById (flight.getId ());
		}
		else if (flight.effdatum ()==otherDate)
		{
			// If otherDate is the same as today, this is not reached.
			preparedFlights.removeById (flight.getId ());
			flightsToday.removeById (flight.getId ());
			flightsOther.replaceOrAdd (flight.getId (), flight);
		}
		else
		{
			// The flight should not be on any list - remove it from all lists
			preparedFlights.removeById (flight.getId ());
			flightsToday.removeById (flight.getId ());
			flightsOther.removeById (flight.getId ());
			interested=false;
		}

		if (interested)
		{
			// By-ID and specific hashes
			objectsByIdHash<Flight> ().insert (flight.getId (), flight);
			updateHashesObjectUpdated<Flight> (flight, old);
		}
	}
}

template<class T> void Cache::handleDbChanged (const DbEvent &event)
{
	switch (event.getType ())
	{
		case DbEvent::typeAdd   : objectAdded      (event.getValue<T> ()); break;
		case DbEvent::typeChange: objectUpdated    (event.getValue<T> ()); break;
		case DbEvent::typeDelete: objectDeleted<T> (event.getId       ()); break;
		// no default
	}
}

void Cache::dbChanged (DbEvent event)
{
	switch (event.getTable ())
	{
		case DbEvent::tableFlights       : handleDbChanged<Flight>       (event); break;
		case DbEvent::tableLaunchMethods : handleDbChanged<LaunchMethod> (event); break;
		case DbEvent::tablePeople        : handleDbChanged<Person>       (event); break;
		case DbEvent::tablePlanes        : handleDbChanged<Plane>        (event); break;
		case DbEvent::tableFlarmNetRecords : handleDbChanged<FlarmNetRecord> (event); break;
		// no default
	}

	// Re-emit the event
	emit changed (event);
}


// **********
// ** Misc **
// **********

void Cache::clear ()
{
	synchronized (dataMutex)
	{
		// Object lists
		planes       .clear ();
		people       .clear ();
		launchMethods.clear ();

		flightsToday.clear (); todayDate=QDate ();
		flightsOther.clear (); otherDate=QDate ();
		preparedFlights.clear ();

		// By-ID hashes
		planesById        .clear ();
		peopleById        .clear ();
		launchMethodsById .clear ();
		flightsById       .clear ();

		// Specific hashes
		clearMultiTypeHashes ();
		clearHashes<Plane> ();
		clearHashes<Person> ();
		clearHashes<LaunchMethod> ();
		clearHashes<Flight> ();
	}
}


// Don't have to instantiate handleDbChanged, objectAdded,
// objectDeleted and objectUpdated as they are only used in this file
