#ifndef MIGRATION_20100216124307_RENAME_COLUMNS_H_
#define MIGRATION_20100216124307_RENAME_COLUMNS_H_

#include "src/db/migration/Migration.h"

/**
 * Renames the columns to proper english names
 */
class Migration_20100216124307_rename_columns: public Migration
{
	public:
		Migration_20100216124307_rename_columns (Interface &interface);
		virtual ~Migration_20100216124307_rename_columns ();

		virtual void up ();
		virtual void down ();

	protected:
		using Migration::renameColumn;
		void renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, Direction direction);
		void renameColumns (Direction direction);
};

#endif

