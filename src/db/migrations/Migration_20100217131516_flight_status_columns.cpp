#include "Migration_20100217131516_flight_status_columns.h"

#include <iostream>

REGISTER_MIGRATION (20100217131516, flight_status_columns)

Migration_20100217131516_flight_status_columns::Migration_20100217131516_flight_status_columns (Interface &interface):
	Migration (interface)
{
}

Migration_20100217131516_flight_status_columns::~Migration_20100217131516_flight_status_columns ()
{
}

void Migration_20100217131516_flight_status_columns::up ()
{
	// Ideally, we would use transactions here, but the DDL statements
	// (add column) terminate the transaction, so it's kind of useless.

	addColumn ("flights", "departed"        , dataTypeBoolean (), "AFTER status");
	addColumn ("flights", "landed"          , dataTypeBoolean (), "AFTER departed");
	addColumn ("flights", "towflight_landed", dataTypeBoolean (), "AFTER landed");

	std::cout << "Updating status flags" << std::endl;
	executeQuery (Query ("UPDATE flights SET "
		"departed"         "=IF(status&1, TRUE, FALSE),"
		"landed"           "=IF(status&2, TRUE, FALSE),"
		"towflight_landed" "=IF(status&4, TRUE, FALSE)"));

	dropColumn ("flights", "status");
}

void Migration_20100217131516_flight_status_columns::down ()
{
	// See #up about transaction

	addColumn ("flights", "status", dataTypeInteger (), "AFTER departed");

	std::cout << "Updating status" << std::endl;
	executeQuery (Query ("UPDATE flights SET status="
		"IF(departed        , 1, 0)+"
		"IF(landed          , 2, 0)+"
		"IF(towflight_landed, 4, 0)"));

	dropColumn ("flights", "departed"        );
	dropColumn ("flights", "landed"          );
	dropColumn ("flights", "towflight_landed");
}
