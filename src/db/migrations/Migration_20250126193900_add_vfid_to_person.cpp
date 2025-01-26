#include "Migration_20250126193900_add_vfid_to_person.h"

REGISTER_MIGRATION (20250126193900, add_vfid_to_person)

Migration_20250126193900_add_vfid_to_person::Migration_20250126193900_add_vfid_to_person (Interface &interface):
	Migration (interface)
{
}

Migration_20250126193900_add_vfid_to_person::~Migration_20250126193900_add_vfid_to_person ()
{
}

void Migration_20250126193900_add_vfid_to_person::up ()
{
    addColumn("people", "vfid", dataTypeLong());
}

void Migration_20250126193900_add_vfid_to_person::down ()
{
    dropColumn("people", "vfid");
}

