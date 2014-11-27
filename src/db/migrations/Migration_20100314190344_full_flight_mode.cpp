#include "Migration_20100314190344_full_flight_mode.h"

#include <iostream>

REGISTER_MIGRATION (20100314190344, full_flight_mode)

Migration_20100314190344_full_flight_mode::Migration_20100314190344_full_flight_mode (Interface &interface):
	Migration (interface)
{
}

Migration_20100314190344_full_flight_mode::~Migration_20100314190344_full_flight_mode ()
{
}

void Migration_20100314190344_full_flight_mode::up ()
{
	changeColumnType ("flights", "mode", dataTypeString ());
	changeColumnType ("flights", "towflight_mode", dataTypeString ());
	std::cout << "Updating flight mode values" << std::endl;
	updateValues (dirUp);
}

void Migration_20100314190344_full_flight_mode::down ()
{
	std::cout << "Updating flight type values" << std::endl;
	updateValues (dirDown);
	changeColumnType ("flights", "mode", dataTypeCharacter ());
}

void Migration_20100314190344_full_flight_mode::updateValue (const QString &oldValue, const QString &newValue, Migration::Direction direction)
{
	switch (direction)
	{
		case dirUp:
			updateColumnValues ("flights", "mode"          , oldValue, newValue);
			updateColumnValues ("flights", "towflight_mode", oldValue, newValue);
			break;
		case dirDown:
			updateColumnValues ("flights", "mode"          , newValue, oldValue);
			updateColumnValues ("flights", "towflight_mode", newValue, oldValue);
			break;
	}
}

void Migration_20100314190344_full_flight_mode::updateValues (Migration::Direction direction)
{
	updateValue ("l", "local"      , direction);
	updateValue ("k", "coming"     , direction);
	updateValue ("g", "leaving"    , direction);
}
