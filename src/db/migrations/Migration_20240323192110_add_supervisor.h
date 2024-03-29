#ifndef MIGRATION_20240323192110_ADD_SUPERVISOR_H_
#define MIGRATION_20240323192110_ADD_SUPERVISOR_H_

#include "src/db/migration/Migration.h"

/**
 * TODO migration description
 */
class Migration_20240323192110_add_supervisor: public Migration
{
	public:
		Migration_20240323192110_add_supervisor (Interface &interface);
		virtual ~Migration_20240323192110_add_supervisor ();

		virtual void up ();
		virtual void down ();
};

#endif

