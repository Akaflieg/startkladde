#include "Migration_20100726124616_add_medical_validity.h"

REGISTER_MIGRATION (20100726124616, add_medical_validity)

Migration_20100726124616_add_medical_validity::Migration_20100726124616_add_medical_validity (Interface &interface):
	Migration (interface)
{
}

Migration_20100726124616_add_medical_validity::~Migration_20100726124616_add_medical_validity ()
{
}

void Migration_20100726124616_add_medical_validity::up ()
{
	addColumn ("people", "medical_validity",       dataTypeDate    ());
	addColumn ("people", "check_medical_validity", dataTypeBoolean ());
}

void Migration_20100726124616_add_medical_validity::down ()
{
	dropColumn ("people", "medical_validity"      );
	dropColumn ("people", "check_medical_validity");
}
