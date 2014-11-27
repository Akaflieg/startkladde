#ifndef MIGRATION_20100427115235_ADD_INDEXES_H_
#define MIGRATION_20100427115235_ADD_INDEXES_H_

#include "src/db/migration/Migration.h"

/**
 * Adds indexes to the tables to speed up both startkladde and sk_web database
 * operations
 */
class Migration_20100427115235_add_indexes: public Migration
{
	public:
		Migration_20100427115235_add_indexes (Interface &interface);
		virtual ~Migration_20100427115235_add_indexes ();

		virtual void up ();
		virtual void down ();

	private:
		void handleIndexes (Migration::Direction direction);
		void handleSimpleIndex (Migration::Direction direction, const QString &table, const QString &column);
		void handleIndex (Migration::Direction direction, const QString &table, const QString &name, const QString &columns);
};

#endif
