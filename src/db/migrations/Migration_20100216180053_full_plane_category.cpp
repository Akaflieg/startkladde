#include "Migration_20100216180053_full_plane_category.h"

REGISTER_MIGRATION (20100216180053, full_plane_category)

Migration_20100216180053_full_plane_category::Migration_20100216180053_full_plane_category (Interface &interface):
	Migration (interface)
{
}

Migration_20100216180053_full_plane_category::~Migration_20100216180053_full_plane_category ()
{
}

void Migration_20100216180053_full_plane_category::up ()
{
	changeColumnType ("planes", "category", dataTypeString ());
	updateValues (dirUp);
}

void Migration_20100216180053_full_plane_category::down ()
{
	updateValues (dirDown);
	changeColumnType ("planes", "category", dataTypeCharacter ());
}

void Migration_20100216180053_full_plane_category::updateValue (const QString &oldValue, const QString &newValue, Migration::Direction direction)
{
	switch (direction)
	{
		case dirUp:   updateColumnValues ("planes", "category", oldValue, newValue); break;
		case dirDown: updateColumnValues ("planes", "category", newValue, oldValue); break;
	}
}

void Migration_20100216180053_full_plane_category::updateValues (Migration::Direction direction)
{
	updateValue ("e", "airplane"    , direction);
	updateValue ("1", "glider"      , direction);
	updateValue ("k", "motorglider" , direction);
	updateValue ("m", "ultralight"  , direction);
	updateValue ("s", "other"       , direction);
}
