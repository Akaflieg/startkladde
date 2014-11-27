#ifndef MIGRATION_20100314190344_FULL_FLIGHT_MODE_H_
#define MIGRATION_20100314190344_FULL_FLIGHT_MODE_H_

#include "src/db/migration/Migration.h"

/**
 * Changes the flights.mode and flights.towflight_mode columns from a character
 * to a string and updates the values.
 */
class Migration_20100314190344_full_flight_mode: public Migration
{
	public:
		Migration_20100314190344_full_flight_mode (Interface &interface);
		virtual ~Migration_20100314190344_full_flight_mode ();

		virtual void up ();
		virtual void down ();

	private:
		void updateValue (const QString &oldValue, const QString &newValue, Direction direction);
		void updateValues (Direction direction);
};

#endif
