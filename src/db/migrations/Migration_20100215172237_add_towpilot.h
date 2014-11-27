#ifndef MIGRATION_20100215172237_ADD_TOWPILOT_H_
#define MIGRATION_20100215172237_ADD_TOWPILOT_H_

#include "src/db/migration/Migration.h"

/**
 * Adds the towpilot columns to the flight table, if they don't exist yet.
 *
 * The towpilot columns were optional before, so this migration transforms
 * the schema to a defined state.
 *
 * This change is not undone by the down migration; see 20100214140000.
 */
class Migration_20100215172237_add_towpilot: public Migration
{
	public:
		Migration_20100215172237_add_towpilot (Interface &interface);
		virtual ~Migration_20100215172237_add_towpilot ();

		virtual void up ();
		virtual void down ();
};

#endif

