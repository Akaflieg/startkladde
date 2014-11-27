#ifndef DBEVENT_H_
#define DBEVENT_H_

#include <QString>
#include <QVariant>

#include "src/db/dbId.h"

/**
 * A description of a change in the database
 *
 * This is implemented with an Table Enum rather than as a template because it
 * is sent as a parameter for a signal and signals can't be templates.
 */
class DbEvent
{
	public:
		// ** Types
		enum Table { tablePeople, tableFlights, tableLaunchMethods, tablePlanes, tableFlarmNetRecords };
		enum Type { typeAdd, typeDelete, typeChange };

		// ** Construction
		DbEvent ();
		DbEvent (Type type, Table table, dbId id, const QVariant &value);

		template<class T> static DbEvent added (const T &object)
		{
			QVariant value;
			value.setValue (object);
			return DbEvent (typeAdd, getTable<T> (), object.getId (), value);
		}

		template<class T> static DbEvent changed (const T &object)
		{
			QVariant value;
			value.setValue (object);
			return DbEvent (typeChange, getTable<T> (), object.getId (), value);
		}

		template<class T> static DbEvent deleted (const T &object)
		{
			QVariant value;
			value.setValue (object);
			return DbEvent (typeDelete, getTable<T> (), object.getId (), value);
		}

		template<class T> static DbEvent deleted (dbId id)
		{
			return DbEvent (typeDelete, getTable<T> (), id, QVariant ());
		}


		// ** Formatting
		QString toString () const;
		static QString typeString (Type type);
		static QString tableString (Table table);

		// ** Property access
		Table    getTable () const { return table; }
		Type     getType  () const { return type ; }
		dbId     getId    () const { return id   ; }
		QVariant getValue () const { return type ; }

		template <class T> T getValue () const { return value.value<T> (); }

		// ** Table methods
		template<class T> bool hasTable () const { return table==getTable<T> (); }
		template<class T> static Table getTable ();

	private:
		Type type;
		Table table;
		dbId id;
		QVariant value;
};

#endif
