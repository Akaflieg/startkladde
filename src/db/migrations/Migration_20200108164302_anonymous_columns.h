#ifndef MIGRATION_20200108164302_ANONYMOUS_COLUMNS_H_
#define MIGRATION_20200108164302_ANONYMOUS_COLUMNS_H_

#include "src/db/migration/Migration.h"

/**
 * TODO migration description
 */
class Migration_20200108164302_anonymous_columns: public Migration
{
	public:
		Migration_20200108164302_anonymous_columns (Interface &interface);
		virtual ~Migration_20200108164302_anonymous_columns ();

		virtual void up ();
		virtual void down ();
};

#endif

