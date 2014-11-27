#ifndef MIGRATION_20100217131516_FLIGHT_STATUS_COLUMNS_H_
#define MIGRATION_20100217131516_FLIGHT_STATUS_COLUMNS_H_

#include "src/db/migration/Migration.h"

/**
 * Splits the flight.status column into flight.departed, flight.landed and
 * flight.towflight_landed
 */
class Migration_20100217131516_flight_status_columns: public Migration
{
	public:
		Migration_20100217131516_flight_status_columns (Interface &interface);
		virtual ~Migration_20100217131516_flight_status_columns ();

		virtual void up ();
		virtual void down ();
};

#endif

