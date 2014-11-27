#ifndef MIGRATION_20100214_140000_H_
#define MIGRATION_20100214_140000_H_

#include "src/db/migration/Migration.h"

/**
 * Creates the database as it was created by pre-migration versions (legacy
 * database).
 *
 * All of the tables and columns will only be created of they do not yet exist.
 * This allows us to treat an empty table the same way as a legacy database.
 *
 * Note that this migration will not exactly recreate the legacy schema. Each
 * of the differences
 *   - is transparent to the old version; that is, the old version of the
 *     software and the version of sk_web that uses the old table runs with
 *     the database created by this migration
 *   - brings some advantage (such as ease of implementation)
 *   - will be fixed in a later migration to create a defined state (except the
 *     column order); that is, a migration will change the database such that
 *     the state is the same regardless of whether the database was created by
 *     this migration or by the legacy version
 *     Note that these migrations will not undo their changes when migrating
 *     down, in order to be consistent on a freshly created (i. e. non-legacy)
 *     database.
 *     Note that the fixes are the first migrations performed. This is why we
 *     don't remove the _temp columns right away.
 *
 * The differences are:
 *   - The storage engine will be InnoDB (was MyISAM before)
 *       - Transparent because no MyISAM specific features were used
 *       - Different because Interface#addTable does not allow
 *         specifying the storage engine
 *       - Fixed in 20100215000000
 *   - The towpilot columns will be created (they were optional before).
 *       - Transparent because if the towpilot was not recorded, the towpilot
 *         columns were ignored
 *       - Different because the towpilot columns may be used
 *       - Fixed in 20100215172237
 *   - The columns flight.editierbar, flight.vereinand person.bwlv will not be
 *     created
 *       - Transparent because the columns were not used
 *       - Different because it reduces work
 *       - Fixed in 20100215211913
 *   - The character set will be utf8 (was latin1 before)
 *       - Transparent because the character set is automatically converted by
 *         MySQL
 *       - Different because Interface#addTable does not allow specifying the
 *         character set; also, no data is lost when migrating down
 *       - Fixed in 20100215215320
 *   - The data types may be different
 *       - Transparent because the data can also be represented in these data
 *         types
 *       - Different because these data types are more suitable (especially
 *         VARCHAR vs. BLOB)
 *       - Fixed in 20100215221900
 *   - The order of the columns may be different
 *       - Transparent because the column order is not relied on
 *       - Different because it makes more sense that way
 *       - Not fixed because the column order is not relied on
 */
class Migration_20100214140000_initial: public Migration
{
	public:
		Migration_20100214140000_initial (Interface &interface);
		virtual ~Migration_20100214140000_initial ();

		virtual void up ();
		virtual void down ();
};

#endif

