#include "Migration_20200108164302_anonymous_columns.h"

REGISTER_MIGRATION (20200108164302, anonymous_columns)

Migration_20200108164302_anonymous_columns::Migration_20200108164302_anonymous_columns (Interface &interface):
	Migration (interface)
{
}

Migration_20200108164302_anonymous_columns::~Migration_20200108164302_anonymous_columns ()
{
}

void Migration_20200108164302_anonymous_columns::up ()
{
    addColumn("flights", "num_crew", dataTypeInteger ());
    addColumn("flights", "num_pax", dataTypeInteger ());
}

void Migration_20200108164302_anonymous_columns::down ()
{
    dropColumn("flights", "num_crew");
    dropColumn("flights", "num_pax");
}

