#include "Migration_20100215215320_convert_to_utf8.h"

#include <iostream>

#include "src/util/qString.h"

REGISTER_MIGRATION (20100215215320, convert_to_utf8)

Migration_20100215215320_convert_to_utf8::Migration_20100215215320_convert_to_utf8 (Interface &interface):
	Migration (interface)
{
}

Migration_20100215215320_convert_to_utf8::~Migration_20100215215320_convert_to_utf8 ()
{
}

void Migration_20100215215320_convert_to_utf8::up ()
{
	changeTable ("person");
	changeTable ("person_temp");
	changeTable ("flugzeug");
	changeTable ("flugzeug_temp");
	changeTable ("flug");
	changeTable ("flug_temp");
	changeTable ("user");
}

void Migration_20100215215320_convert_to_utf8::down ()
{
	// Don't change back
}

void Migration_20100215215320_convert_to_utf8::changeTable (const QString &name)
{
	std::cout << "Changing table " << name << std::endl;

	executeQuery (
			QString ("ALTER TABLE %1 CONVERT TO CHARACTER SET utf8 COLLATE utf8_unicode_ci")
		.arg (name)
	);
}
