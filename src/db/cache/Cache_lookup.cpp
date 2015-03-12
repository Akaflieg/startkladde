#include "Cache.h"

#include <cassert>

#include <QtCore/QDebug>
#include <QList>
#include <QString>
#include <QStringList>

#include "src/concurrent/synchronized.h"
#include "src/model/Flight.h"
#include "src/model/LaunchMethod.h"
#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/flarm/flarmNet/FlarmNetRecord.h"
#include "src/container/SortedSet_impl.h"
#include "src/i18n/notr.h"

// ******************
// ** Object lists **
// ******************

/**
 * Makes a copy of the object list
 *
 * We don't return a (const) reference to the list because the list
 * itself is not thread safe and accesses to the list have to
 * synchronized. Copying the list is fast if the list is not modified
 * thanks to Qt implicit sharing.
 */
template<class T> EntityList<T> Cache::getObjects () const
{
	synchronizedReturn (dataMutex, objectList<T> ());
}


EntityList<Plane> Cache::getPlanes ()
{
	synchronizedReturn (dataMutex, planes);
}

QList<Plane> Cache::getPlanesSortedByUsage()
{
    synchronizedReturn (dataMutex, planesSortedByUsage);
}

EntityList<Person> Cache::getPeople ()
{
	synchronizedReturn (dataMutex, people);
}

EntityList<LaunchMethod> Cache::getLaunchMethods ()
{
	synchronizedReturn (dataMutex, launchMethods);
}

EntityList<FlarmNetRecord> Cache::getFlarmNetRecords ()
{
	synchronizedReturn (dataMutex, flarmNetRecords);
}


EntityList<Flight> Cache::getFlightsToday (bool includeTowflights)
{
	if (includeTowflights)
	{
		QList<Flight> flights=getFlightsToday (false).getList ();
		flights+=Flight::makeTowflights (flights, *this);
		return flights;
	}
	else
	{
		synchronizedReturn (dataMutex, flightsToday);
	}
}

EntityList<Flight> Cache::getFlightsOther ()
{
	synchronizedReturn (dataMutex, flightsOther);
}

EntityList<Flight> Cache::getPreparedFlights (bool includeTowflights)
{
	if (includeTowflights)
	{
		QList<Flight> flights=getPreparedFlights (false).getList ();

		// Add the prepared towflights. Note that, contrary to the flying
		// flights, a prepared towflight always corresponds to a prepared
		// flight. Therefore, we can base the list of prepared towflights on the
		// list of prepared flights (the list of flying towflights must be based
		// an the list of all of today's flights, not just the flying flights).
		flights+=Flight::makeTowflights (flights, *this);
		return flights;
	}
	else
	{
		synchronizedReturn (dataMutex, preparedFlights);
	}
}

EntityList<Flight> Cache::getFlyingFlights (bool includeTowflights)
{
	// Add flying flights to the list. Prepared flights can also be flying if
	// their mode is "coming". Don't directly access the preparedFlights
	// property - the access must be synchronized. We don't add the towflights
	// for prepared flights - coming flights don't have a launch method, and
	// therefore no towflights.

	QList<Flight> candidates;
	candidates.append (getFlightsToday    (includeTowflights).getList ());
	candidates.append (getPreparedFlights (false            ).getList ());

	// Only keep flying flights
	EntityList<Flight> result;
	foreach (const Flight &flight, candidates)
		if (flight.isFlying ())
			result.append (flight);

	// Return the list
	return result;
}

QDate Cache::getTodayDate ()
{
	synchronizedReturn (dataMutex, todayDate);
}

QDate Cache::getOtherDate ()
{
	synchronizedReturn (dataMutex, otherDate);
}

EntityList<Flight> Cache::getAllKnownFlights ()
{
	synchronizedReturn (dataMutex, flightsToday+flightsOther+preparedFlights);
}


// ************************
// ** Individual objects **
// ************************

/**
 * Gets an object from the cache; the database is not accessed.
 *
 * @param id the ID of the object
 * @return a copy of the object
 * @throw NotFoundException if the object is not found or id is invalid
 */
template<class T> T Cache::getObject (dbId id)
{
	if (idInvalid (id)) throw NotFoundException (id);

	synchronized (dataMutex)
	{
		const QHash<dbId, T> &hash=objectsByIdHash<T> ();

		if (!hash.contains (id)) throw NotFoundException (id);
		return hash.value (id);
	}

	assert (!notr ("Not returned yet"));
	throw NotFoundException (id);
}

/**
 * Gets an object from the cache; the database is not accessed.
 *
 * @param id the ID of the object
 * @return a newly allocated copy of the object (the caller takes
 *         ownership) or NULL the object is not found or id is invalid
 */
template<class T> T* Cache::getNewObject (dbId id)
{
	// Not synchronized, we're not accessing any data
	try
	{
		return new T (getObject<T> (id));
	}
	catch (NotFoundException &ex)
	{
		return NULL;
	}
}

/**
 * Determines if an object exists in the cache; the database is not
 * accessed.
 *
 * @param id the ID of the object
 * @return true if the object exists, or false if not
 */
template<class T> bool Cache::objectExists (dbId id, T& typeSpec)
{
    (void)typeSpec;

	synchronized (dataMutex)
		return objectsByIdHash<T> ().contains (id);

    return false;
}

template<class T> QList<T> Cache::getObjects (const QList<dbId> &ids, bool ignoreNotFound)
{
	QList<T> result;

	foreach (dbId id, ids)
	{
		try
		{
			result.append (getObject<T> (id));
		}
		catch (NotFoundException &)
		{
			if (!ignoreNotFound) throw;
		}
	}

	return result;
}


// *************************
// ** Objects by property **
// *************************

dbId Cache::getPlaneIdByRegistration (const QString &registration)
{
	synchronized (dataMutex)
	{
		if (!planeIdsByRegistration.contains (registration.toLower ()))
			return invalidId;

		return planeIdsByRegistration.value (registration.toLower ());
	}

	assert (!notr ("Not returned yet"));
	return invalidId;
}

QList<dbId> Cache::getPlaneIdsByFlarmId (const QString &flarmId)
{
	synchronizedReturn (dataMutex, planeIdsByFlarmId.values (flarmId));
}

// FIXME remove - the Flarm ID need not be unique, so all functions calling this
// should handle multiple results (or ignore it explicitly)
dbId Cache::getPlaneIdByFlarmId (const QString &flarmId)
{
	synchronized (dataMutex)
	{
		if (!planeIdsByFlarmId.contains (flarmId))
			return invalidId;

		return planeIdsByFlarmId.value (flarmId);
	}

	assert (!notr ("Not returned yet"));
	return invalidId;
}

dbId Cache::getFlarmNetRecordIdByFlarmId (const QString &flarmId)
{
	synchronized (dataMutex)
	{
		if (!flarmNetRecordIdsByFlarmId.contains (flarmId))
			return invalidId;

		return flarmNetRecordIdsByFlarmId.value (flarmId);
	}

	assert (!notr ("Not returned yet"));
	return invalidId;
}

QList<dbId> Cache::getPersonIdsByName (const QString &lastName, const QString &firstName)
{
	QPair<QString, QString> pair (lastName.toLower (), firstName.toLower ());
	synchronizedReturn (dataMutex, personIdsByName.values (pair));
}

/**
 * Returns the ID of the person with the given last and first name
 * (case insensitively) if there is exactly one such person, or an
 * invalid id if threre are multiple or no such people
 *
 * @param lastName the last name of the person (the case is ignored)
 * @param firstName the first name of the person (the case is ignored)
 * @return the ID of the person, or an invalid ID
 */
dbId Cache::getUniquePersonIdByName (const QString &lastName, const QString &firstName)
{
	const QList<dbId> personIds=getPersonIdsByName (lastName, firstName);

	if (personIds.size ()==1) return personIds.at (0);
	return invalidId;
}

QList<dbId> Cache::getPersonIdsByLastName (const QString &lastName)
{
	synchronizedReturn (dataMutex, personIdsByLastName.values (lastName.toLower ()));
}

QList<dbId> Cache::getPersonIdsByFirstName (const QString &firstName)
{
	synchronizedReturn (dataMutex, personIdsByFirstName.values (firstName.toLower ()));
}

dbId Cache::getLaunchMethodByType (LaunchMethod::Type type) const
{
	synchronized (dataMutex)
	{
		const QList<dbId> ids=launchMethodIdsByType.values (type);
		if (ids.size ()>0) return ids.at (0);
	}

	return invalidId;
}

QList<dbId> Cache::getFlarmNetRecordIds () const {
	synchronizedReturn (dataMutex,  flarmNetRecordIdsByFlarmId.values());
}

/*
QList<FlarmNetRecord> Cache::getFlarmNetRecords () const {
	synchronizedReturn (dataMutex,  flarmNetRecords);
}
*/

// ******************
// ** String lists **
// ******************

QStringList Cache::getPlaneRegistrations ()
{
	synchronizedReturn (dataMutex, planeRegistrations.toQList ());
}

QStringList Cache::getPersonFirstNames ()
{
	synchronizedReturn (dataMutex, personFirstNames.toQList ());
}

QStringList Cache::getPersonFirstNames (const QString &lastName)
{
	QStringList firstNames;
	synchronized (dataMutex)
		firstNames=firstNamesByLastName.values (lastName.toLower ());

	firstNames.sort ();
	return firstNames;
}

QStringList Cache::getPersonLastNames ()
{
	synchronizedReturn (dataMutex, personLastNames.toQList ());
}

QStringList Cache::getPersonLastNames (const QString &firstName)
{
	QStringList lastNames;
	synchronized (dataMutex)
		lastNames=lastNamesByFirstName.values (firstName.toLower ());

	lastNames.sort ();
	return lastNames;
}

QStringList Cache::getLocations ()
{
	synchronizedReturn (dataMutex, locations.toQList ());
}

QStringList Cache::getAccountingNotes ()
{
	synchronizedReturn (dataMutex, accountingNotes.toQList ());
}

QStringList Cache::getPlaneTypes ()
{
	synchronizedReturn (dataMutex, planeTypes.toQList ());
}

QStringList Cache::getClubs ()
{
	synchronizedReturn (dataMutex, clubs.toQList ());
}


// *******************
// ** Object flying **
// *******************

dbId Cache::planeFlying (dbId id)
{
	synchronized (dataMutex)
	{
		// Only use the flights of today
		foreach (const Flight &flight, flightsToday.getList ())
		{
			// Plane is the plane of this flight?
			if (flight.getPlaneId ()==id && flight.isFlying ())
				return flight.getId ();

			// Plane is the towplane of this flight?
			if (flight.isTowplaneFlying () && flight.isAirtow (*this) &&
					flight.effectiveTowplaneId (*this)==id)
				return flight.getId ();
		}
	}

	return invalidId;
}

dbId Cache::personFlying (dbId id)
{
	synchronized (dataMutex)
	{
		// Only use the flights of today
		foreach (const Flight &flight, flightsToday.getList ())
		{
			if (
				(flight.isFlying         () && flight.getPilotId    ()==id) ||
				(flight.isFlying         () && flight.getCopilotId  ()==id) ||
				(flight.isTowplaneFlying () && flight.getTowpilotId ()==id))
				return flight.getId ();
		}
	}

	return invalidId;
}


// ****************************
// ** Template instantiation **
// ****************************

#define INSTANTIATE_TEMPLATES(T) \
	template T  Cache::getObject    (dbId id); \
	template T *Cache::getNewObject (dbId id); \
	template QList<T> Cache::getObjects (const QList<dbId> &ids, bool ignoreNotFound); \
    template bool Cache::objectExists (dbId id, T& typeSpec); \
	// Empty line

#define INSTANTIATE_NON_FLIGHT_TEMPLATES(T) \
	template EntityList<T> Cache::getObjects () const; \
	// Empty line

INSTANTIATE_TEMPLATES (Person        )
INSTANTIATE_TEMPLATES (Plane         )
INSTANTIATE_TEMPLATES (Flight        )
INSTANTIATE_TEMPLATES (LaunchMethod  )
INSTANTIATE_TEMPLATES (FlarmNetRecord)

INSTANTIATE_NON_FLIGHT_TEMPLATES (Person        )
INSTANTIATE_NON_FLIGHT_TEMPLATES (Plane         )
INSTANTIATE_NON_FLIGHT_TEMPLATES (LaunchMethod  )
INSTANTIATE_NON_FLIGHT_TEMPLATES (FlarmNetRecord)

#undef INSTANTIATE_TEMPLATES
#undef INSTANTIATE_NON_FLIGHT_TEMPLATES
