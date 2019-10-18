#ifndef MIGRATION_20191018135747_ADD_VFID_H_
#define MIGRATION_20191018135747_ADD_VFID_H_

#include "src/db/migration/Migration.h"

/**
 * TODO migration description
 */
class Migration_20191018135747_add_vfid: public Migration
{
	public:
		Migration_20191018135747_add_vfid (Interface &interface);
		virtual ~Migration_20191018135747_add_vfid ();

		virtual void up ();
		virtual void down ();
};

#endif

