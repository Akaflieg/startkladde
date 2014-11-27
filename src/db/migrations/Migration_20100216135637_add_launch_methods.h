#ifndef MIGRATION_20100216135637_ADD_LAUNCH_METHODS_H_
#define MIGRATION_20100216135637_ADD_LAUNCH_METHODS_H_

#include <QString>
#include <QList>

#include "src/db/migration/Migration.h"

class LaunchMethod;

/**
 * Adds the launch methods table (and imports the launch methods from the
 * configuration file)
 *
 * Also imports old (configured) launch methods from ~/.startkladde.conf or
 * ./startkladde.conf:
 *   startart <id>, <type>, <towplane_registration>, <name>, <short_name>, <keyboard_shortcut>, <log_string>, <person_required>
 *
 */
class Migration_20100216135637_add_launch_methods: public Migration
{
	public:
		Migration_20100216135637_add_launch_methods (Interface &interface);
		virtual ~Migration_20100216135637_add_launch_methods ();

		virtual void up ();
		virtual void down ();

	private:
		static QString effectiveConfigFileName ();
		static QList<LaunchMethod> readConfiguredLaunchMethods ();
};

#endif

