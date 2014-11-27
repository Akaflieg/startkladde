#ifndef MIGRATION_20100215000000_CHANGE_TO_INNODB_H_
#define MIGRATION_20100215000000_CHANGE_TO_INNODB_H_

#include "src/db/migration/Migration.h"

/**
 * Changes the storage engine used for the database tables to InnoDB.
 *
 * The storage engine may have been MyISAM before. Unlike MyISAM, InnoDB
 * supports transactions.
 *
 * This is the first migration performed so we have transactions in the later
 * migrations.
 *
 * This change is not undone by the down migration; see 20100214140000.
 */
class Migration_20100215000000_change_to_innodb: public Migration
{
	public:
		Migration_20100215000000_change_to_innodb (Interface &interface);
		virtual ~Migration_20100215000000_change_to_innodb ();

		virtual void up ();
		virtual void down ();

	protected:
		virtual void changeTable (const QString &name);
};

#endif

