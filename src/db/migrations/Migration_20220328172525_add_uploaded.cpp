#include "Migration_20220328172525_add_uploaded.h"

REGISTER_MIGRATION (20220328172525, add_uploaded)

Migration_20220328172525_add_uploaded::Migration_20220328172525_add_uploaded (Interface &interface):
	Migration (interface)
{
}

Migration_20220328172525_add_uploaded::~Migration_20220328172525_add_uploaded ()
{
}

void Migration_20220328172525_add_uploaded::up ()
{
    addColumn("flights", "uploaded", dataTypeBoolean ());
    executeQuery (Query("UPDATE flights SET uploaded = true"));
}

void Migration_20220328172525_add_uploaded::down ()
{
    dropColumn("flights", "uploaded");
}

