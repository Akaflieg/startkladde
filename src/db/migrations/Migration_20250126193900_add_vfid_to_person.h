#ifndef MIGRATION_20250126193900_ADD_VFID_TO_PERSON_H_
#define MIGRATION_20250126193900_ADD_VFID_TO_PERSON_H_

#include "src/db/migration/Migration.h"

/**
 * TODO migration description
 */
class Migration_20250126193900_add_vfid_to_person: public Migration
{
	public:
        Migration_20250126193900_add_vfid_to_person (Interface &interface);
        virtual ~Migration_20250126193900_add_vfid_to_person ();

		virtual void up ();
		virtual void down ();
};

#endif

