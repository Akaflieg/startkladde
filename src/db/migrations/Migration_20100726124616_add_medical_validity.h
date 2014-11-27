#ifndef MIGRATION_20100726124616_ADD_MEDICAL_VALIDITY_H_
#define MIGRATION_20100726124616_ADD_MEDICAL_VALIDITY_H_

#include "src/db/migration/Migration.h"

/**
 * TODO migration description
 */
class Migration_20100726124616_add_medical_validity: public Migration
{
	public:
		Migration_20100726124616_add_medical_validity (Interface &interface);
		virtual ~Migration_20100726124616_add_medical_validity ();

		virtual void up ();
		virtual void down ();
};

#endif

