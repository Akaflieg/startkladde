#include "Migration_20100427115235_add_indexes.h"

REGISTER_MIGRATION (20100427115235, add_indexes)

Migration_20100427115235_add_indexes::Migration_20100427115235_add_indexes (Interface &interface):
	Migration (interface)
{
}

Migration_20100427115235_add_indexes::~Migration_20100427115235_add_indexes ()
{
}

void Migration_20100427115235_add_indexes::up ()
{
	handleIndexes (Migration::dirUp);
}

void Migration_20100427115235_add_indexes::down ()
{
	handleIndexes (Migration::dirDown);
}

void Migration_20100427115235_add_indexes::handleIndexes (Migration::Direction direction)
{
	handleSimpleIndex (direction, "planes", "registration");
	handleSimpleIndex (direction, "planes", "club"        );


	handleSimpleIndex (direction, "people", "club"   );
	handleSimpleIndex (direction, "people", "club_id");


	handleSimpleIndex (direction, "users", "username" );
	handleSimpleIndex (direction, "users", "person_id");


	handleSimpleIndex (direction, "flights", "plane_id"                  );
	handleSimpleIndex (direction, "flights", "pilot_id"                  );
	handleSimpleIndex (direction, "flights", "copilot_id"                );
	handleSimpleIndex (direction, "flights", "launch_method_id"          );
	handleSimpleIndex (direction, "flights", "towplane_id"               );
	handleSimpleIndex (direction, "flights", "towpilot_id"               );

	handleSimpleIndex (direction, "flights", "type"                      );
	handleSimpleIndex (direction, "flights", "mode"                      );
	handleSimpleIndex (direction, "flights", "towflight_mode"            );

	handleSimpleIndex (direction, "flights", "departed"                  );
	handleSimpleIndex (direction, "flights", "landed"                    );
	handleSimpleIndex (direction, "flights", "towflight_landed"          );
	handleIndex (direction, "flights", "status_index", "departed,landed,towflight_landed");

	handleSimpleIndex (direction, "flights", "departure_time"            );
	handleSimpleIndex (direction, "flights", "landing_time"              );
	handleSimpleIndex (direction, "flights", "towflight_landing_time"    );
	handleSimpleIndex (direction, "flights", "departure_location"        );
	handleSimpleIndex (direction, "flights", "landing_location"          );
	handleSimpleIndex (direction, "flights", "towflight_landing_location");

	handleSimpleIndex (direction, "flights", "accounting_notes"          );
}

void Migration_20100427115235_add_indexes::handleIndex (Migration::Direction direction, const QString &table, const QString &name, const QString &columns)
{
	switch (direction)
	{
		case dirUp:   createIndex (IndexSpec (table, name, columns)); break;
		case dirDown: dropIndex              (table, name)          ; break;
	}
}

void Migration_20100427115235_add_indexes::handleSimpleIndex (Migration::Direction direction, const QString &table, const QString &column)
{
	handleIndex (direction, table, column+"_index", column);
}
