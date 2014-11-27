#ifndef MIGRATION_20100216190338_FULL_FLIGHT_TYPE_H_
#define MIGRATION_20100216190338_FULL_FLIGHT_TYPE_H_

#include "src/db/migration/Migration.h"

/**
 * Changes the flights.type column from an integer to a string and updates the
 * values.
 */
class Migration_20100216190338_full_flight_type: public Migration
{
	public:
		Migration_20100216190338_full_flight_type (Interface &interface);
		virtual ~Migration_20100216190338_full_flight_type ();

		virtual void up ();
		virtual void down ();

	protected:
		void updateValue (int oldValue, const QString &newValue, Direction direction);
		void updateValues (Direction direction);
};

#endif

