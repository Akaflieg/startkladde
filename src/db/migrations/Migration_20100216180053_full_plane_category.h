#ifndef MIGRATION_20100216180053_FULL_PLANE_CATEGORY_H_
#define MIGRATION_20100216180053_FULL_PLANE_CATEGORY_H_

#include "src/db/migration/Migration.h"

/**
 * Changes the planes.category column from a character to a string and updates
 * the values
 */
class Migration_20100216180053_full_plane_category: public Migration
{
	public:
		Migration_20100216180053_full_plane_category (Interface &interface);
		virtual ~Migration_20100216180053_full_plane_category ();

		virtual void up ();
		virtual void down ();

	protected:
		void updateValue (const QString &oldValue, const QString &newValue, Direction direction);
		void updateValues (Direction direction);
};

#endif

