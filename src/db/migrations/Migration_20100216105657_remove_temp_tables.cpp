#include "Migration_20100216105657_remove_temp_tables.h"

#include <iostream>
#include "src/util/qString.h"

REGISTER_MIGRATION (20100216105657, remove_temp_tables)

Migration_20100216105657_remove_temp_tables::Migration_20100216105657_remove_temp_tables (Interface &interface):
	Migration (interface)
{
}

Migration_20100216105657_remove_temp_tables::~Migration_20100216105657_remove_temp_tables ()
{
}

void Migration_20100216105657_remove_temp_tables::up ()
{
	removeTempTable ("person");
	removeTempTable ("flugzeug");
	removeTempTable ("flug");
}

void Migration_20100216105657_remove_temp_tables::down ()
{
	createTempTable ("person");
	createTempTable ("flugzeug");
	createTempTable ("flug");
}

void Migration_20100216105657_remove_temp_tables::removeTempTable (const QString &name)
{
	QString temp_name=name+"_temp";

	std::cout << QString ("Copying entries from from %2 to %1")
		.arg (name, temp_name) << std::endl;

	executeQuery (
		QString ("INSERT IGNORE INTO %1 SELECT * FROM %2")
		.arg (name, temp_name)
		);

	dropTable (temp_name);
}

void Migration_20100216105657_remove_temp_tables::createTempTable (const QString &name)
{
	// Rename the table to _temp and create an empty proper table, thus moving
	// the entries to the _temp table.
	// The entries are moved because legacy sk_web (that is, the Rails sk_web
	// version using the legacy database) only uses the _temp table. This also
	// means that all entries will by considered editable by legacy versions.

	QString temp_name=name+"_temp";

	renameTable (name, temp_name);
	createTableLike (temp_name, name);
}
