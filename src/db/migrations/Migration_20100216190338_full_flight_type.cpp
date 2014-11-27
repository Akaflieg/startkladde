#include "Migration_20100216190338_full_flight_type.h"

#include <iostream>

REGISTER_MIGRATION (20100216190338, full_flight_type)

Migration_20100216190338_full_flight_type::Migration_20100216190338_full_flight_type (Interface &interface):
	Migration (interface)
{
}

Migration_20100216190338_full_flight_type::~Migration_20100216190338_full_flight_type ()
{
}

void Migration_20100216190338_full_flight_type::up ()
{
	changeColumnType ("flights", "type", dataTypeString ());
	std::cout << "Updating flight type values" << std::endl;
	updateValues (dirUp);
}

void Migration_20100216190338_full_flight_type::down ()
{
	std::cout << "Updating flight type values" << std::endl;
	updateValues (dirDown);
	changeColumnType ("flights", "type", dataTypeInteger ());
}

void Migration_20100216190338_full_flight_type::updateValue (int oldValue, const QString &newValue, Migration::Direction direction)
{
	switch (direction)
	{
		case dirUp:   updateColumnValues ("flights", "type", oldValue, newValue); break;
		case dirDown: updateColumnValues ("flights", "type", newValue, oldValue); break;
	}
}

void Migration_20100216190338_full_flight_type::updateValues (Migration::Direction direction)
{
	updateValue (1, "?"             , direction); // None
	updateValue (2, "normal"        , direction);
	updateValue (3, "training_2"    , direction);
	updateValue (4, "training_1"    , direction);
	updateValue (7, "tow"           , direction);
	updateValue (6, "guest_private" , direction);
	updateValue (8, "guest_external", direction);
}
