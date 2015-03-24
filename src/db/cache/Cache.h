/*
 * Cache.h
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#ifndef CACHE_H_
#define CACHE_H_

#include <QObject>
#include <QDate>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QHash>

#include "src/db/dbId.h"
#include "src/model/LaunchMethod.h" // Required for LaunchMethod::Type
#include "src/model/objectList/EntityList.h"
#include "src/db/event/DbEvent.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"
#include "src/container/SortedSet.h"
#include "src/container/SkMultiHash.h"

class Flight;
class Person;
class Plane;
class FlarmNetRecord;

template<class T> class EntityList;

class Database;

/**
 * A cache for a Database
 *
 * Caches the raw object as well as data generated from the object
 * lists (e. g. the list of clubs) and other data explicitly read from
 * the database (e. g. the list of locations).
 *
 * The Cache tracks changes to the Database by the Event::Events emitted by
 * the database. It also emits Event::Events after the cache contents
 * change. Classes using the cache should listen for Event::Events from the
 * cache rather than from the database.
 *
 * The QLists returned by the methods of this class are implicitly
 * shared by Qt, so the data is not copied until the lists are modified
 * or accessed by operator[] or a non-const iterator. If a list is not
 * to be modified, it is recommended to declare it as const (e. g. const
 * List<Plane>=cache.getPlanes ()) to prevent accidental
 * modifications which would cause the list data to be copied.
 *
 * This class is thread safe, provided that the database is thread safe
 * (that is, accesses to the database are not synchronized). Note,
 * however, that the mutex u *   - allow specifying an "exclude" value to selectDistinctColumns (what for?)
 * sed to protect access do the data is
 * recursive, that is, care should be taken when accessing the cache
 * from a function that is called by a method of the cache. This also
 * includes directly called signals.
 */
class Cache: public QObject
{
	Q_OBJECT

	public:
		class NotFoundException: public std::exception
		{
			public:
				NotFoundException (dbId id): id (id) {}
				dbId id;
		};

		// *** Construction
		Cache (Database &db);
		virtual ~Cache ();

		// *** Properties
		Database &getDatabase ();

		// *** Generic refreshing
		template<class T> void refreshObjects (OperationMonitorInterface monitor=OperationMonitorInterface::null);

		// *** Specific refreshing
        void refreshStatic          (OperationMonitorInterface monitor=OperationMonitorInterface::null);
		void refreshPlanes          (OperationMonitorInterface monitor=OperationMonitorInterface::null);
		void refreshPeople          (OperationMonitorInterface monitor=OperationMonitorInterface::null);
		void refreshLaunchMethods   (OperationMonitorInterface monitor=OperationMonitorInterface::null);
		void refreshFlarmNetRecords (OperationMonitorInterface monitor=OperationMonitorInterface::null);
		void refreshFlightsToday    (OperationMonitorInterface monitor=OperationMonitorInterface::null);
		void refreshFlightsOther    (OperationMonitorInterface monitor=OperationMonitorInterface::null);
		void refreshPreparedFlights (OperationMonitorInterface monitor=OperationMonitorInterface::null);
		void refreshLocations       (OperationMonitorInterface monitor=OperationMonitorInterface::null);
		void refreshAccountingNotes (OperationMonitorInterface monitor=OperationMonitorInterface::null);
		void refreshAll             (OperationMonitorInterface monitor=OperationMonitorInterface::null);

		void refreshFlights (OperationMonitorInterface monitor=OperationMonitorInterface::null);
		void fetchFlightsOther (QDate date, OperationMonitorInterface monitor=OperationMonitorInterface::null);

		// *** Misc
		void clear ();

		// ***** Lookup (implemented in Cache_lookup.cpp)

		// *** Object lists
		template<class T> EntityList<T> getObjects () const;

		EntityList<Plane> getPlanes ();
        QList<Plane> getPlanesSortedByUsage ();
		EntityList<Person> getPeople ();
        QList<Person> getPeopleSortedByFrequency ();
		EntityList<LaunchMethod> getLaunchMethods ();
		EntityList<FlarmNetRecord> getFlarmNetRecords ();

		// TODO should probably not be EntityLists
		EntityList<Flight> getFlightsToday (bool includeTowflights);
		EntityList<Flight> getFlightsOther ();
		EntityList<Flight> getPreparedFlights (bool includeTowflights);
		EntityList<Flight> getFlyingFlights (bool includeTowflights);
		QDate getTodayDate ();
		QDate getOtherDate ();
		EntityList<Flight> getAllKnownFlights ();

		// *** Individual objects
		template<class T> T getObject (dbId id);
        template<class T> T* getNewObject (dbId id);
        template<class T> bool objectExists (dbId id, T& typeSpec);
		template<class T> QList<T> getObjects (const QList<dbId> &ids, bool ignoreNotFound);

		// *** Objects by property
		dbId getPlaneIdByRegistration (const QString &registration);
		dbId getPlaneIdByFlarmId (const QString &flarmId);
		QList<dbId> getPlaneIdsByFlarmId (const QString &flarmId);
		dbId getFlarmNetRecordIdByFlarmId (const QString &flarmId);
		QList<dbId> getPersonIdsByName (const QString &lastName, const QString &firstName);
		dbId getUniquePersonIdByName (const QString &lastName, const QString &firstName);
		QList<dbId> getPersonIdsByFirstName (const QString &firstName);
		QList<dbId> getPersonIdsByLastName (const QString &lastName);
		dbId getLaunchMethodByType (LaunchMethod::Type type) const;
		QList<dbId> getFlarmNetRecordIds () const;

        // *** Statistics
        int getNumberOfTakeoffsByPersonId(dbId) const;
        int getNumberOfHoursByPersonId(dbId) const;

		// *** String lists
		QStringList getPlaneRegistrations ();
		QStringList getPersonLastNames ();
		QStringList getPersonLastNames (const QString &firstName);
		QStringList getPersonFirstNames ();
		QStringList getPersonFirstNames (const QString &lastName);
		QStringList getLocations ();
		QStringList getAccountingNotes ();
		QStringList getPlaneTypes ();
		QStringList getClubs ();

		// *** Object flying
		dbId planeFlying (dbId id);
		dbId personFlying (dbId id);

	signals:
		void changed (DbEvent event);

	protected:
		// *** Data access helpers
		template<class T> const EntityList<T> &objectList () const;
		template<class T>       EntityList<T> &objectList ()      ;
		template<class T> const QHash<dbId, T> &objectsByIdHash () const;
		template<class T>       QHash<dbId, T> &objectsByIdHash ()      ;

		// *** Generic refreshing
		void refreshFlightsOf (const QString &description, const QDate &date, EntityList<Flight> &targetList, QDate *targetDate, OperationMonitorInterface monitor);

		// *** Change handling - generic
		template<class T> void handleDbChanged (const DbEvent &event);

		template<class T> void objectAdded (const T &object);
		template<class T> void objectDeleted (dbId id);
		template<class T> void objectUpdated (const T &object);

		// ***** Hash updates (implemented in Cache_hashUpdates.cpp)

		void clearMultiTypeHashes ();

		// These methods have to be specialized
		// A pointer to the old object is passed because it is possible
		// that it does not exist in the cache (it shouldn't happen,
		// but we cannot rule it out and we still want to perform the
		// operation); this is also the reason why we still pass the
		// ID.
		template<class T> void clearHashes ();
		template<class T> void updateHashesObjectAdded (const T &object);
		template<class T> void updateHashesObjectDeleted (dbId id, const T *oldObject);
		template<class T> void updateHashesObjectUpdated (const T &object, const T *oldObject);

	protected slots:
		// *** Change handling - generic
		void dbChanged (DbEvent event);

	private:
		// *** Database
		Database &db;

		// *** Data
		// Note: when adding something here, also handle it in the
		// methods defined in Cache_hashUpdates.

		// Object lists - could also use AutomaticEntityList (but
		// updating methods would have to be changed)
		EntityList<Plane> planes;
        QList<Plane> planesSortedByUsage;
		EntityList<Person> people;
        QList<Person> peopleSortedByFrequency;
		EntityList<LaunchMethod> launchMethods;
		EntityList<FlarmNetRecord> flarmNetRecords;

		// Flight lists - several lists, therefore cannot use
		// AutomaticEntityList (should have AutomaticFlightList which
		// stores a date)
		EntityList<Flight> flightsToday;
		QDate todayDate;
		EntityList<Flight> flightsOther;
		QDate otherDate;
		EntityList<Flight> preparedFlights;

		// String lists
		// Clubs and plane types are generated from other data.
		// Locations and accounting notes are retrieved directly from
		// the database, but will still be added individually when a
		// flight is created.
		SortedSet<QString> locations;
		SortedSet<QString> accountingNotes;
		SortedSet<QString> clubs;
		SortedSet<QString> planeTypes;
		SortedSet<QString> planeRegistrations;
		SortedSet<QString> personLastNames;
		SortedSet<QString> personFirstNames;

		// Hashes by ID
		// QHash is used rather than QMap because it provides
		// "significantly faster lookups" which is important here
		QHash<dbId   , Plane       >        planesById;
		QHash<dbId   , Person      >        peopleById;
		QHash<dbId   , LaunchMethod> launchMethodsById;
		QHash<dbId   , Flight      >       flightsById;
        QHash<dbId   , FlarmNetRecord>     flarmNetRecordsById;

		// Specific hashes
		// The keys of these hashes are lower case; names are QPair
		// (lastName, firstName) (also in lower case).
		//
		// Note: if, in the corresponding updateHashesObjectDeleted method
		// (defined in Cache_hashUpdates.cpp) a value is not removed from the
		// cache, attention must be paid to keep the cache duplicate-free on
		// update (which is done as a delete and an add). For SortedSets, this
		// is not an issue; for MultiHashes, the insertUnique method of
		// SkMultiHash can be used.
		// A better data structure would be a UniqueMultiHash or a
		// Hash<SortedSet>.
		QMultiHash<QString, dbId> planeIdsByRegistration; // key is lower case
		QMultiHash<QString, dbId> planeIdsByFlarmId;
		QMultiHash<LaunchMethod::Type, dbId> launchMethodIdsByType;
		SkMultiHash<QString, QString> lastNamesByFirstName; // key is lower case
		SkMultiHash<QString, QString> firstNamesByLastName; // key is lower case
		QMultiHash<QString, dbId> personIdsByLastName; // key is lower case
		QMultiHash<QString, dbId> personIdsByFirstName; // key is lower case
		QMultiHash<QPair<QString, QString>, dbId> personIdsByName; // key is lower case
		QMultiHash<QString, dbId> flarmNetRecordIdsByFlarmId;
        // "Rough" means not being updated on flight creation/deletion
        // but only on application startup
        QHash<dbId, int> roughNumberOfFlightsByPlaneIdLastYear;
        QHash<dbId, int> roughNumberOfFlightsByPersonIdLastYear;
        QHash<dbId, int> numberOfHoursByPersonIdLastHalfYear;
        QHash<dbId, int> numberOfFlightsByPersonIdLastHalfYear;

		// Concurrency
		// Improvement: use rw mutex and separate locks for flights, people...
		/** Locks accesses to data of this Cache */
		mutable QMutex dataMutex;

};

#endif
