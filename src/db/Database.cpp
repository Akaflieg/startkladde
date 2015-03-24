/*
 * Notes:
 *   - the write operations (create, update, delete) are wrapped in a
 *     transaction because when the operation is canceled forcefully (by using
 *     interface#cancelConnection), we cannot determine whether the operation
 *     was performed or not, so the cache may be invalid. Using a transaction
 *     hopefully reduces the "critical" time where this may happen.
 *     TODO: this means we cannot use a transaction around multiple operations
 *
 * TODO: the selection frontends, value lists and additional properties should
 * probably replaced with queries (potentially with after filter).
 *
 * TODO: Generic methods should be separate from specific methods (selection
 * frontends, value lists, additional properties) and the name of the generic
 * methods class should reflect its function (Orm or similar).
 */
#include "Database.h"

#include <iostream>
#include <cassert>

#include <QDateTime>

#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/model/Flight.h"
#include "src/model/LaunchMethod.h"
#include "src/flarm/flarmNet/FlarmNetRecord.h"
#include "src/util/qString.h"
#include "src/util/qList.h"
#include "src/db/Query.h"
#include "src/db/result/Result.h"
#include "src/util/qDate.h" // TODO remove
#include "src/i18n/notr.h"

// ******************
// ** Construction **
// ******************

Database::Database (Interface &interface):
	interface (interface)
{
}


Database::~Database()
{
}


// ****************
// ** Connection **
// ****************

void Database::cancelConnection ()
{
	interface.cancelConnection ();
}

// *********
// ** ORM **
// *********

template<class T> QList<T> Database::getObjects (const Query &condition)
{
	Query query=Query::select (T::dbTableName (), T::selectColumnList ())
		.condition (condition);

	return T::createListFromResult (*interface.executeQueryResult (query));
}

template<class T> int Database::countObjects (const Query &condition)
{
	Query query=Query::count (T::dbTableName ()).condition (condition);
	return interface.countQuery (query);
}

template<class T> bool Database::objectExists (const Query &query)
{
	return countObjects<T> (query)>0;
}

template<class T> bool Database::objectExists (dbId id)
{
	return objectExists<T> (Query (notr ("id=?")).bind (id));
}

template<class T> T Database::getObject (dbId id)
{
	Query query=Query::select (T::dbTableName (), T::selectColumnList ())
		.condition (Query (notr ("id=?")).bind (id));

	QSharedPointer<Result> result=interface.executeQueryResult (query);

	if (!result->next ()) throw NotFoundException ();

	return T::createFromResult (*result);
}

// TODO there should be a method that only deletes and object if it is not used
// (atomically, by extending the condition). This should be the default for
// deleting objects.
template<class T> bool Database::deleteObject (dbId id)
{
	Query query=Query (notr ("DELETE FROM %1 WHERE ID=?"))
		.arg (T::dbTableName ()).bind (id);

	// Wrap the operation into a transaction, see top of file
	interface.transaction ();
	QSharedPointer<Result> result=interface.executeQueryResult (query);
	interface.commit ();

	emit dbEvent (DbEvent::deleted<T> (id));

	return result->numRowsAffected ()>0;
}

template<class T> int Database::deleteObjects (const QList<dbId> &ids)
{
	// Don't perform the query with an empty list, it will fail.
	if (ids.isEmpty ())
		return 0;

	Query query=
		Query (notr ("DELETE FROM %1 WHERE "))
			.arg (T::dbTableName ())
		+Query::valueInListCondition (notr ("id"), convertType<QVariant> (ids));

	// Wrap the operation into a transaction, see top of file
	interface.transaction ();
	QSharedPointer<Result> result=interface.executeQueryResult (query);
	interface.commit ();

	foreach (dbId id, ids)
		emit dbEvent (DbEvent::deleted<T> (id));

	return result->numRowsAffected ()>0;
}

/**
 * The id of the object is ignored and overwritten.
 *
 * @param object
 * @return
 */
template<class T> dbId Database::createObject (T &object)
{
	Query query=Query (notr ("INSERT INTO %1 (%2) values (%3)"))
		.arg (T::dbTableName (), T::insertColumnList (), T::insertPlaceholderList ());
	object.bindValues (query);

	// Wrap the operation into a transaction, see top of file
	interface.transaction ();
	QSharedPointer<Result> result=interface.executeQueryResult (query);
	interface.commit ();

	object.setId (result->lastInsertId ().toLongLong ());

	if (idValid (object.getId ()))
		emit dbEvent (DbEvent::added (object));

	return object.getId ();
}

template<class T> void Database::createObjects (QList<T> &objects, OperationMonitorInterface monitor)
{
	int num=objects.size ();
	monitor.progress (0, num);

	for (int i=0; i<num; ++i)
	{
		createObject (objects[i]);
		monitor.progress (i+1, num);
	}
}

template<class T> bool Database::updateObject (const T &object)
{
	// Use REPLACE INTO instead of UPDATE so in case the object does not exist
	// (e. g. because of an inconsistent cache), it will be created.
	Query query=Query (notr ("REPLACE INTO %1 (id,%2) values (?,%3)"))
		.arg (T::dbTableName (), T::insertColumnList (), T::insertPlaceholderList ());
	query.bind (object.getId ());
	object.bindValues (query);

	// Wrap the operation into a transaction, see top of file
	interface.transaction ();
	QSharedPointer<Result> result=interface.executeQueryResult (query);
	interface.commit ();

	emit dbEvent (DbEvent::changed (object));

	return result->numRowsAffected ()>0;
}

template<class T> QList<T> Database::getObjects ()
{
	return getObjects<T> (Query ());
}

template<class T> int Database::countObjects ()
{
	return countObjects<T> (Query ());
}


// *******************
// ** Very specific **
// *******************

QStringList Database::listLocations ()
{
	return interface.listStrings (Query::selectDistinctColumns (
		Flight::dbTableName (),
		QStringList () << notr ("departure_location") << notr ("landing_location") << notr ("towflight_landing_location"),
		true));
}

QStringList Database::listAccountingNotes ()
{
	return interface.listStrings (Query::selectDistinctColumns (
		Flight::dbTableName (),
		notr ("accounting_notes"),
		true));
}

QStringList Database::listClubs ()
{
	return interface.listStrings (Query::selectDistinctColumns (
		QStringList () << Plane::dbTableName() << Person::dbTableName (),
		notr ("club"),
		true));
}

QStringList Database::listPlaneTypes ()
{
	return interface.listStrings (Query::selectDistinctColumns (
		Plane::dbTableName (),
		notr ("type"),
		true));
}


QList<Flight> Database::getPreparedFlights ()
{
	// The correct criterion for prepared flights is:
	// !(happened)
	// also known as
	// !((departs_here and departed) or (lands_here and landed))
	//
	// Resolving the flight mode, we get:
	// !( (local and (departed or landed)) or (leaving and departed) or (coming and landed) )
	//
	// Applying de Morgan (the MySQL query optimizer has a better chance of
	// using an index with AND clauses):
	// !( (local and !(!departed AND !landed)) or (leaving and departed) or (coming and landed) )
	//
	// Applying de Morgan to the outer clause (may not be necessary):
	// !(local and !(!departed AND !landed)) and !(leaving and departed) and !(coming and landed)
	//
	// Note that we test for =0 or !=0 explicitly rather than evaluating the
	// values as booleans (i. e. 'where departed=0' instead of 'where
	// departed') because evaluating as booleans prevents using the index

	// TODO to Flight
	// TODO multi-bind
	Query condition (notr ("!(mode=? AND !(departed=0 AND landed=0)) AND !(mode=? AND departed!=0) AND !(mode=? AND landed!=0)"));
	condition.bind (Flight::modeToDb (Flight::modeLocal  ));
	condition.bind (Flight::modeToDb (Flight::modeLeaving));
	condition.bind (Flight::modeToDb (Flight::modeComing ));

	return getObjects<Flight> (condition);
}

QList<Flight> Database::getFlightsDate (QDate date)
{
	Query condition=Flight::dateSupersetCondition (date);

	QList<Flight> candidates=getObjects<Flight> (condition);
	QList<Flight> flights=Flight::dateSupersetFilter (candidates, date);

	return flights;
}

template<class T> bool Database::objectUsed (dbId id)
{
	(void)id;
	// Return true for safety; specialize for specific classes
	return true;
}

template<> bool Database::objectUsed<Person> (dbId id)
{
	// ATTENTION: make sure that DbManager::mergePeople correspondents to this
	// method

	// A person may be referenced by a flight
	if (objectExists<Flight> (Flight::referencesPersonCondition (id))) return true;

	// A person may be referenced by a user (although we don't have a user
	// model here, only in sk_web)
	if (interface.countQuery (
		Query::count (notr ("users"), Query (notr ("person_id=?")).bind (id))
	)) return true;

	return false;
}

template<> bool Database::objectUsed<Plane> (dbId id)
{
	// Launch methods reference planes by registration, which we don't have
	// to check.

	// A plane may be reference by a flight
	return objectExists<Flight> (Flight::referencesPlaneCondition (id));
}

template<> bool Database::objectUsed<LaunchMethod> (dbId id)
{
	// A launch method may be reference by a flight
	return objectExists<Flight> (Flight::referencesLaunchMethodCondition (id));
}

template<> bool Database::objectUsed<Flight> (dbId id)
{
	(void)id;

	// Flights are never used
	return false;
}

QHash<dbId, int> Database::flightsPerPlane(QDate from, QDate to)
{
    Query query ("SELECT plane_id, COUNT(*) FROM flights WHERE landed=1 AND departed=1 AND date(departure_time) >= ? AND date(departure_time) <= ? GROUP BY plane_id");
    query.bind(from);
    query.bind(to);
    QSharedPointer<Result> r = interface.executeQueryResult(query);
    QHash<dbId, int> map;

    while (r->next())
        map.insert(r->value(0).toInt(), r->value(1).toInt());

    return map;
}

QHash<dbId, int> Database::flightsPerPerson(QDate from, QDate to)
{
    Query query ("SELECT pilot_id, COUNT(*) FROM flights WHERE landed=1 AND departed=1 AND date(departure_time) >= ? AND date(departure_time) <= ? GROUP BY pilot_id");
    query.bind(from);
    query.bind(to);
    QSharedPointer<Result> r = interface.executeQueryResult(query);
    QHash<dbId, int> map;

    while (r->next())
        map.insert(r->value(0).toInt(), r->value(1).toInt());

    Query query2 ("SELECT copilot_id, COUNT(*) FROM flights WHERE landed=1 AND departed=1 AND date(departure_time) >= ? AND date(departure_time) <= ? GROUP BY copilot_id");
    query2.bind(from);
    query2.bind(to);
    r = interface.executeQueryResult(query2);

    while (r->next())
    {
        dbId id = r->value(0).toInt();
        int nr = r->value(1).toInt();
        if (map.contains(id)) {
            map.insert(id, map.value(id) + nr);
        } else {
            map.insert(id, nr);
        }
    }

    return map;
}

QHash<dbId, int> Database::flightsPerPersonAsPic(QDate from, QDate to)
{
    Query query ("SELECT pilot_id, COUNT(*) FROM flights WHERE landed=1 AND departed=1 AND date(departure_time) >= ? AND date(departure_time) <= ? GROUP BY pilot_id");
    query.bind(from);
    query.bind(to);
    QSharedPointer<Result> r = interface.executeQueryResult(query);
    QHash<dbId, int> map;

    while (r->next())
        map.insert(r->value(0).toInt(), r->value(1).toInt());

    return map;
}

QHash<dbId, int> Database::hoursPerPersonAsPic(QDate from, QDate to)
{
    Query query ("SELECT pilot_id, SUM(TIMESTAMPDIFF(MINUTE, departure_time, landing_time)) FROM flights WHERE landed=1 AND departed=1 AND date(departure_time) >= ? AND date(departure_time) <= ? GROUP BY pilot_id");
    query.bind(from);
    query.bind(to);
    QSharedPointer<Result> r = interface.executeQueryResult(query);
    QHash<dbId, int> map;

    while (r->next())
        map.insert(r->value(0).toInt(), r->value(1).toInt() / 60);

    return map;
}


// **********
// ** Misc **
// **********

void Database::emitDbEvent (DbEvent event)
{
	// Hack for merging people, invoked by our friend DbManager::mergePeople
	emit dbEvent (event);
}

// ***************************
// ** Method instantiations **
// ***************************

// Instantiate the method templates
// Classes have to provide:
//   - ::dbTableName ();
//   - ::QString selectColumnList (); // TODO return queries directly?
//   - ::createFromQuery (const Result &result); // TODO change to create
//   - ::insertColumnList ();
//   - ::insertPlaceholderList ();
//   - bindValues (QSqlQuery &q) const;
//   - ::createListFromQuery (Result &result); // TODO change to createList

#define INSTANTIATE_TEMPLATES(T) \
	template QList<T> Database::getObjects       (const Query &condition); \
	template int      Database::countObjects<T>  (const Query &condition); \
	template bool     Database::objectExists<T>  (dbId id); \
	template T        Database::getObject        (dbId id); \
	template bool     Database::deleteObject<T>  (dbId id); \
	template int      Database::deleteObjects<T> (const QList<dbId> &id); \
	template dbId     Database::createObject     (T &object); \
	template void     Database::createObjects    (QList<T> &objects, OperationMonitorInterface monitor); \
	template bool     Database::updateObject     (const T &object); \
	template QList<T> Database::getObjects  <T>  (); \
	template int      Database::countObjects<T>  (); \
	template bool     Database::objectUsed<T>    (dbId id);

	// Empty line

INSTANTIATE_TEMPLATES (Person      )
INSTANTIATE_TEMPLATES (Plane       )
INSTANTIATE_TEMPLATES (Flight      )
INSTANTIATE_TEMPLATES (LaunchMethod)
INSTANTIATE_TEMPLATES (FlarmNetRecord)

#undef INSTANTIATE_TEMPLATES
