#ifndef MIGRATION_20191018135747_ADD_VFID_H_
#define MIGRATION_20220328172525_ADD_UPLOADED_H_

#include "src/db/migration/Migration.h"

/**
 * TODO migration description
 */
class Migration_20220328172525_add_uploaded: public Migration
{
	public:
		Migration_20220328172525_add_uploaded (Interface &interface);
		virtual ~Migration_20220328172525_add_uploaded ();

		virtual void up ();
		virtual void down ();
};

#endif

