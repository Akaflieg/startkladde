#include "Migration_20120612130803_add_flarm_id.h"

REGISTER_MIGRATION (20120612130803, add_flarm_id)

Migration_20120612130803_add_flarm_id::Migration_20120612130803_add_flarm_id (Interface &interface):
	Migration (interface)
{
}

Migration_20120612130803_add_flarm_id::~Migration_20120612130803_add_flarm_id ()
{
}

void Migration_20120612130803_add_flarm_id::up ()
{
	addColumn ("planes" , "flarm_id", dataTypeString16 ());
	addColumn ("flights", "flarm_id", dataTypeString16 ());
}

void Migration_20120612130803_add_flarm_id::down ()
{
	dropColumn ("planes" , "flarm_id");
	dropColumn ("flights", "flarm_id");
}
