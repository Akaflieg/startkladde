#ifndef MIGRATION_20100216171107_ADD_COMMENTS_TO_USERS_H_
#define MIGRATION_20100216171107_ADD_COMMENTS_TO_USERS_H_

#include "src/db/migration/Migration.h"

/**
 * Adds a comments column to the user table
 */
class Migration_20100216171107_add_comments_to_users: public Migration
{
	public:
		Migration_20100216171107_add_comments_to_users (Interface &interface);
		virtual ~Migration_20100216171107_add_comments_to_users ();

		virtual void up ();
		virtual void down ();
};

#endif

