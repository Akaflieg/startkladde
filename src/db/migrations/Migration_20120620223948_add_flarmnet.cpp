#include "Migration_20120620223948_add_flarmnet.h"

REGISTER_MIGRATION (20120620223948, add_flarmnet)

Migration_20120620223948_add_flarmnet::Migration_20120620223948_add_flarmnet (Interface &interface):
	Migration (interface)
{
}

Migration_20120620223948_add_flarmnet::~Migration_20120620223948_add_flarmnet ()
{
}

void Migration_20120620223948_add_flarmnet::up ()
{
	createTable  ("flarmnet", true); // Creates the id column
	addColumn ("flarmnet", "flarm_id",     dataTypeString16 (), "NOT NULL", true);
	addColumn ("flarmnet", "registration", dataTypeString16 (), "NOT NULL", true);
	addColumn ("flarmnet", "callsign",     dataTypeString16 (), "", true);
	addColumn ("flarmnet", "owner",        dataTypeString (),   "", true);
	addColumn ("flarmnet", "airfield",     dataTypeString (),   "", true);
	addColumn ("flarmnet", "type",         dataTypeString (),   "", true);
	addColumn ("flarmnet", "frequency",    dataTypeString16 (), "", true);
}

void Migration_20120620223948_add_flarmnet::down ()
{
	dropTable ("flarmnet");
}

