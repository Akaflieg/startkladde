#include "Migration_20100216124307_rename_columns.h"

REGISTER_MIGRATION (20100216124307, rename_columns)

Migration_20100216124307_rename_columns::Migration_20100216124307_rename_columns (Interface &interface):
	Migration (interface)
{
}

Migration_20100216124307_rename_columns::~Migration_20100216124307_rename_columns ()
{
}

void Migration_20100216124307_rename_columns::up ()
{
	renameColumns (dirUp);
}

void Migration_20100216124307_rename_columns::down ()
{
	renameColumns (dirDown);
}

void Migration_20100216124307_rename_columns::renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, Migration::Direction direction)
{
	switch (direction)
	{
		case dirUp:   renameColumn (table, oldName, newName, type); break;
		case dirDown: renameColumn (table, newName, oldName, type); break;
	}
}

void Migration_20100216124307_rename_columns::renameColumns (Direction direction)
{
	// *** People
	// id is not changed
	renameColumn ("people", "nachname"  , "last_name" , dataTypeString (), direction);
	renameColumn ("people", "vorname"   , "first_name", dataTypeString (), direction);
	renameColumn ("people", "verein"    , "club"      , dataTypeString (), direction);
	renameColumn ("people", "spitzname" , "nickname"  , dataTypeString (), direction);
	renameColumn ("people", "vereins_id", "club_id"   , dataTypeString (), direction);
	renameColumn ("people", "bemerkung" , "comments"  , dataTypeString (), direction);

	// *** Planes
	// id is not changed
	renameColumn ("planes", "kennzeichen"           , "registration"              ,  dataTypeString    (), direction);
	renameColumn ("planes", "verein"                , "club"                      ,  dataTypeString    (), direction);
	renameColumn ("planes", "sitze"                 , "num_seats"                 ,  dataTypeInteger   (), direction);
	renameColumn ("planes", "typ"                   , "type"                      ,  dataTypeString    (), direction);
	renameColumn ("planes", "gattung"               , "category"                  ,  dataTypeCharacter (), direction);
	renameColumn ("planes", "wettbewerbskennzeichen", "callsign"                  ,  dataTypeString    (), direction);
	renameColumn ("planes", "bemerkung"             , "comments"                  ,  dataTypeString    (), direction);

	// *** Flights
	// id is not changed
	renameColumn ("flights", "flugzeug"             , "plane_id"                  ,  dataTypeId        (), direction);
	renameColumn ("flights", "pilot"                , "pilot_id"                  ,  dataTypeId        (), direction);
	renameColumn ("flights", "begleiter"            , "copilot_id"                ,  dataTypeId        (), direction);
	// Flight settings
	renameColumn ("flights", "typ"                  , "type"                      ,  dataTypeInteger   (), direction);
	renameColumn ("flights", "modus"                , "mode"                      ,  dataTypeCharacter (), direction);
	// Departure and landing
	renameColumn ("flights", "status"               , "status"                    ,  dataTypeInteger   (), direction);
	renameColumn ("flights", "startart"             , "launch_method_id"          ,  dataTypeId        (), direction);
	renameColumn ("flights", "startort"             , "departure_location"        ,  dataTypeString    (), direction);
	renameColumn ("flights", "zielort"              , "landing_location"          ,  dataTypeString    (), direction);
	renameColumn ("flights", "anzahl_landungen"     , "num_landings"              ,  dataTypeInteger   (), direction);
	renameColumn ("flights", "startzeit"            , "departure_time"            ,  dataTypeDatetime  (), direction);
	renameColumn ("flights", "landezeit"            , "landing_time"              ,  dataTypeDatetime  (), direction);
	// Towflight
	renameColumn ("flights", "towplane"             , "towplane_id"               ,  dataTypeId        (), direction);
	renameColumn ("flights", "modus_sfz"            , "towflight_mode"            ,  dataTypeCharacter (), direction);
	renameColumn ("flights", "zielort_sfz"          , "towflight_landing_location",  dataTypeString    (), direction);
	renameColumn ("flights", "land_schlepp"         , "towflight_landing_time"    ,  dataTypeDatetime  (), direction);
	renameColumn ("flights", "towpilot"             , "towpilot_id"               ,  dataTypeId        (), direction);
	// Incomplete names
	renameColumn ("flights", "pnn"                  , "pilot_last_name"           ,  dataTypeString    (), direction);
	renameColumn ("flights", "pvn"                  , "pilot_first_name"          ,  dataTypeString    (), direction);
	renameColumn ("flights", "bnn"                  , "copilot_last_name"         ,  dataTypeString    (), direction);
	renameColumn ("flights", "bvn"                  , "copilot_first_name"        ,  dataTypeString    (), direction);
	renameColumn ("flights", "tpnn"                 , "towpilot_last_name"        ,  dataTypeString    (), direction);
	renameColumn ("flights", "tpvn"                 , "towpilot_first_name"       ,  dataTypeString    (), direction);
	// Comments
	renameColumn ("flights", "bemerkung"            , "comments"                  ,  dataTypeString    (), direction);
	renameColumn ("flights", "abrechnungshinweis"   , "accounting_notes"          ,  dataTypeString    (), direction);

	// *** Users
	// username, password, perm_club_admin, perm_read_flight_db and club are not changed
	renameColumn ("users", "person"                 , "person_id"                 ,  dataTypeId        (), direction);
}
