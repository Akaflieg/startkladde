#ifndef MIGRATION_20100216105657_REMOVE_TEMP_TABLES_H_
#define MIGRATION_20100216105657_REMOVE_TEMP_TABLES_H_

#include "src/db/migration/Migration.h"

/**
 * Removes the _temp tables.
 *
 * Values from the _temp tables are copied to the proper tables. Entries in the
 * proper tables take precedence over the corresponding entries from the _temp
 * table, that is, we don't overwrite entries with the same ID.
 *
 * After copying, the _temp tables are removed.
 */
class Migration_20100216105657_remove_temp_tables: public Migration
{
	public:
		Migration_20100216105657_remove_temp_tables (Interface &interface);
		virtual ~Migration_20100216105657_remove_temp_tables ();

		virtual void up ();
		virtual void down ();

	protected:
		void removeTempTable (const QString &name);
		void createTempTable (const QString &name);
};

#endif

