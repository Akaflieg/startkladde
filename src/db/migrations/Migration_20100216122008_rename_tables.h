#ifndef MIGRATION_20100216122008_RENAME_TABLES_H_
#define MIGRATION_20100216122008_RENAME_TABLES_H_

#include "src/db/migration/Migration.h"

/**
 * Renames the tables to pluralized english names
 */
class Migration_20100216122008_rename_tables: public Migration
{
	public:
		Migration_20100216122008_rename_tables (Interface &interface);
		virtual ~Migration_20100216122008_rename_tables ();

		virtual void up ();
		virtual void down ();

	protected:
		using Migration::renameTable;
		void renameTable (const QString &oldName, const QString &newName, Direction direction);
		void renameTables (Direction direction);
};

#endif

