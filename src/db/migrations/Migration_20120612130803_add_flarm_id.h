#ifndef MIGRATION_20120612130803_ADD_FLARM_ID_H_
#define MIGRATION_20120612130803_ADD_FLARM_ID_H_

#include "src/db/migration/Migration.h"

/**
 * TODO migration description
 */
class Migration_20120612130803_add_flarm_id: public Migration
{
	public:
		Migration_20120612130803_add_flarm_id (Interface &interface);
		virtual ~Migration_20120612130803_add_flarm_id ();

		virtual void up ();
		virtual void down ();
};

#endif

