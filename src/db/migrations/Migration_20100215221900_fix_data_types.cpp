#include "Migration_20100215221900_fix_data_types.h"

REGISTER_MIGRATION (20100215221900, fix_data_types)

Migration_20100215221900_fix_data_types::Migration_20100215221900_fix_data_types (Interface &interface):
	Migration (interface)
{
}

Migration_20100215221900_fix_data_types::~Migration_20100215221900_fix_data_types ()
{
}

void Migration_20100215221900_fix_data_types::up ()
{
	changePeopleTable ("person");
	changePeopleTable ("person_temp");
	changePlanesTable ("flugzeug");
	changePlanesTable ("flugzeug_temp");
	changeFlightsTable ("flug");
	changeFlightsTable ("flug_temp");
	changeUsersTable ("user");
}

void Migration_20100215221900_fix_data_types::down ()
{
	// Don't change back
}

void Migration_20100215221900_fix_data_types::changePeopleTable (const QString &name)
{
	changeColumnType (name, "nachname"  , dataTypeString ());
	changeColumnType (name, "vorname"   , dataTypeString ());
	changeColumnType (name, "verein"    , dataTypeString ());
	changeColumnType (name, "spitzname" , dataTypeString ());
	changeColumnType (name, "vereins_id", dataTypeString ());
	changeColumnType (name, "bemerkung" , dataTypeString ());
}

void Migration_20100215221900_fix_data_types::changePlanesTable (const QString &name)
{
	changeColumnType (name, "kennzeichen",            dataTypeString    ());
	changeColumnType (name, "verein",                 dataTypeString    ());
	changeColumnType (name, "sitze",                  dataTypeInteger   ());
	changeColumnType (name, "typ",                    dataTypeString    ());
	changeColumnType (name, "gattung",                dataTypeCharacter ());
	changeColumnType (name, "wettbewerbskennzeichen", dataTypeString    ());
	changeColumnType (name, "bemerkung",              dataTypeString    ());
}

void Migration_20100215221900_fix_data_types::changeFlightsTable (const QString &name)
{
	changeColumnType (name, "flugzeug",           dataTypeId        ());
	changeColumnType (name, "pilot",              dataTypeId        ());
	changeColumnType (name, "begleiter",          dataTypeId        ());
	// Flight settings
	changeColumnType (name, "typ",                dataTypeInteger   ());
	changeColumnType (name, "modus",              dataTypeCharacter ());
	// Departure and landing
	changeColumnType (name, "status",             dataTypeInteger   ());
	changeColumnType (name, "startart",           dataTypeId        ());
	changeColumnType (name, "startort",           dataTypeString    ());
	changeColumnType (name, "zielort",            dataTypeString    ());
	changeColumnType (name, "anzahl_landungen",   dataTypeInteger   ());
	changeColumnType (name, "startzeit",          dataTypeDatetime  ());
	changeColumnType (name, "landezeit",          dataTypeDatetime  ());
	// Towflight
	changeColumnType (name, "towplane",           dataTypeId        ());
	changeColumnType (name, "modus_sfz",          dataTypeCharacter ());
	changeColumnType (name, "zielort_sfz",        dataTypeString    ());
	changeColumnType (name, "land_schlepp",       dataTypeDatetime  ());
	changeColumnType (name, "towpilot",           dataTypeId        ());
	// Incomplete names
	changeColumnType (name, "pvn",                dataTypeString    ());
	changeColumnType (name, "pnn",                dataTypeString    ());
	changeColumnType (name, "bvn",                dataTypeString    ());
	changeColumnType (name, "bnn",                dataTypeString    ());
	changeColumnType (name, "tpvn",               dataTypeString    ());
	changeColumnType (name, "tpnn",               dataTypeString    ());
	// Comments                                                     ()
	changeColumnType (name, "bemerkung",          dataTypeString    ());
	changeColumnType (name, "abrechnungshinweis", dataTypeString    ());
}

void Migration_20100215221900_fix_data_types::changeUsersTable (const QString &name)
{
	changeColumnType (name, "username",            dataTypeString  (), "NOT NULL");
	changeColumnType (name, "password",            dataTypeString  ());
	changeColumnType (name, "perm_club_admin",     dataTypeBoolean ());
	changeColumnType (name, "perm_read_flight_db", dataTypeBoolean ());
	changeColumnType (name, "club",                dataTypeString  ());
	changeColumnType (name, "person",              dataTypeId      ());
}
