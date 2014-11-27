#include "Migration_20100215211913_drop_old_columns.h"

REGISTER_MIGRATION (20100215211913, drop_old_columns)

Migration_20100215211913_drop_old_columns::Migration_20100215211913_drop_old_columns (Interface &interface):
	Migration (interface)
{
}

Migration_20100215211913_drop_old_columns::~Migration_20100215211913_drop_old_columns ()
{
}

void Migration_20100215211913_drop_old_columns::up ()
{
	dropColumnAndTemp ("flug"       , "editierbar");
	dropColumnAndTemp ("flug"       , "verein"    );
	dropColumnAndTemp ("person"     , "bwlv"      );
}

void Migration_20100215211913_drop_old_columns::down ()
{
	// Don't change back
}

void Migration_20100215211913_drop_old_columns::dropColumnAndTemp (const QString &table, const QString &name)
{
	dropColumn (table        , name, true);
	dropColumn (table+"_temp", name, true);
}
