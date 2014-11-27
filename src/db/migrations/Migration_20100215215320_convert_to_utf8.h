#ifndef MIGRATION_20100215215320_CONVERT_TO_UTF8_H_
#define MIGRATION_20100215215320_CONVERT_TO_UTF8_H_

#include "src/db/migration/Migration.h"

/**
 * Makes sure that the character set is is utf8.
 *
 * The conversion is done by the MySQL server. Also the character set for
 * strings is stored with the strings, so we don't have to make any assumptions
 * about the old character set.
 *
 * This change is not undone by the down migration; see 20100214140000.
 */
class Migration_20100215215320_convert_to_utf8: public Migration
{
	public:
		Migration_20100215215320_convert_to_utf8 (Interface &interface);
		virtual ~Migration_20100215215320_convert_to_utf8 ();

		virtual void up ();
		virtual void down ();

	protected:
		virtual void changeTable (const QString &name);
};

#endif

