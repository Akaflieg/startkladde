#include "Migration_20240323192110_add_supervisor.h"

REGISTER_MIGRATION (20240323192110, add_supervisor)

Migration_20240323192110_add_supervisor::Migration_20240323192110_add_supervisor (Interface &interface):
	Migration (interface)
{
}

Migration_20240323192110_add_supervisor::~Migration_20240323192110_add_supervisor ()
{
}

void Migration_20240323192110_add_supervisor::up ()
{
    addColumn("flights", "supervisor_id", dataTypeInteger());
}

void Migration_20240323192110_add_supervisor::down ()
{
    dropColumn("flights", "supervisor_id");
}

