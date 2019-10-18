#include "Migration_20191018135747_add_vfid.h"

REGISTER_MIGRATION (20191018135747, add_vfid)

Migration_20191018135747_add_vfid::Migration_20191018135747_add_vfid (Interface &interface):
	Migration (interface)
{
}

Migration_20191018135747_add_vfid::~Migration_20191018135747_add_vfid ()
{
}

void Migration_20191018135747_add_vfid::up ()
{
    addColumn("flights", "vfid", dataTypeInteger ());
}

void Migration_20191018135747_add_vfid::down ()
{
    dropColumn("flights", "vfid");
}

