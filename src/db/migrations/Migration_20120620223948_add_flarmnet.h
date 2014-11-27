#ifndef MIGRATION_20120620223948_ADD_FLARMNET_H_
#define MIGRATION_20120620223948_ADD_FLARMNET_H_

#include "src/db/migration/Migration.h"

/**
 * add the new table flarmnet
 */
class Migration_20120620223948_add_flarmnet: public Migration
{
	public:
		Migration_20120620223948_add_flarmnet (Interface &interface);
		virtual ~Migration_20120620223948_add_flarmnet ();

		virtual void up ();
		virtual void down ();
};

#endif

