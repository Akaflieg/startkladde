/*
 * TODO:
 *   - add template generators a la added<T> (id) and
 *     added (flight)
 *   - pass a copy of the object, via QSharedPointer (nb Thread safety,
 *     probably make a copy on get)
 *   - rename changed to updated
 */
#include "DbEvent.h"

#include <iostream>
#include <cassert>

#include "src/i18n/notr.h"

class Plane;
class Flight;
class Person;
class LaunchMethod;
class FlarmNetRecord;

/**
 * This stinks, but sending signals across threads requires queued events,
 * queued events require qRegisterMetaType and qRegisterMetaType requires
 * a default constructor.
 *
 * In order not to have to introduce "dummy" values for the enums, we
 * construct an arbitrary DbEvent here.
 *
 * But, as it turns out, that default contructor is never called. So we can
 * disallow use of this constructor by raising an assertion.
 */
DbEvent::DbEvent ():
	type (typeChange), table (tablePeople), id (invalidId)
{
	assert (!notr ("DbEvent default constructor called"));
}

DbEvent::DbEvent (Type type, Table table, dbId id, const QVariant &value):
	type (type), table (table), id (id), value (value)
{
}



QString DbEvent::toString () const
{
	return qnotr ("db_event (type: %1, table: %2, id: %3)")
		.arg (typeString (type), tableString (table)).arg (id);
}

QString DbEvent::typeString (DbEvent::Type type)
{
	switch (type)
	{
		case typeAdd    : return notr ("add");
		case typeDelete : return notr ("delete");
		case typeChange : return notr ("change");
		// no default
	}

	assert (!notr ("Unhandled type"));
	return notr ("?");
}

QString DbEvent::tableString (DbEvent::Table table)
{
	switch (table)
	{
		case tablePeople:		return notr ("people");
		case tableFlights:		return notr ("flights");
		case tableLaunchMethods: 	return notr ("launch methods");
		case tablePlanes:		return notr ("planes");
		case tableFlarmNetRecords:	return notr ("flarmnet");
	}

	assert (!notr ("Unhandled table"));
	return notr ("?");
}

// Specialize
template<> DbEvent::Table DbEvent::getTable<Flight>       () { return tableFlights      ; }
template<> DbEvent::Table DbEvent::getTable<Plane>        () { return tablePlanes       ; }
template<> DbEvent::Table DbEvent::getTable<Person>       () { return tablePeople       ; }
template<> DbEvent::Table DbEvent::getTable<LaunchMethod> () { return tableLaunchMethods; }
template<> DbEvent::Table DbEvent::getTable<FlarmNetRecord> () { return tableFlarmNetRecords; }
