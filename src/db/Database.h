#ifndef DATABASE_H_
#define DATABASE_H_

#include <QString>
#include <QList>
#include <QtSql>
#include <QHash>
#include <QStringList>
#include <QSqlError>
#include <QObject>

#include "src/db/dbId.h"
#include "src/db/interface/Interface.h"
#include "src/db/event/DbEvent.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"

class Flight;


/**
 * Methods for object-level access to the database
 *
 * This class implements the Greater ORM, which consists of:
 *   - Object CRUDLEC (create, read, update, delete, list, exists, count)
 *   - Selection frontends (e. g. getFlightsDate)
 *   - additional methods, like objectUsed
 *
 * The CRUDLEC methods are templates which are instantiated for the
 * relevant classes in the .cpp file.
 *
 * This class is thread safe, provided that the Interface used (by passing
 * to the constructor) is thread safe. This applies to ThreadSafeInterface,
 * but not to DefaultInterface).
 * Any restrictions applying to the Interface used also apply to this
 * class; for example, a Database instance using a DefaultInterface may
 * only be used in the thread that created the DefaultInterface.
 *
 * Note that we do not use the ENUM SQL type because it is not supported by
 * SQLite.
 */
class Database: public QObject
{
	Q_OBJECT;

	// Hack required for merging people
	friend class DbManager;

	public:
		// *** Data types
		class NotFoundException {};

		// *** Construction
		Database (Interface &interface);
		virtual ~Database ();

		// *** ORM
		// Template functions, instantiated for the relevant classes
		template<class T> QList<T> getObjects (const Query &condition);
		template<class T> int countObjects (const Query &condition);
		template<class T> bool objectExists (const Query &condition);
		template<class T> bool objectExists (dbId id);
		template<class T> T getObject (dbId id);
		template<class T> bool deleteObject (dbId id);
		template<class T> int deleteObjects (const QList<dbId> &ids);
		template<class T> dbId createObject (T &object);
		template<class T> void createObjects (QList<T> &objects, OperationMonitorInterface monitor=OperationMonitorInterface::null);
		template<class T> bool updateObject (const T &object);

		// We could use a default parameter for the corresponding methods
		// taking a Query&, but that would require files using this method
		// to include Query, which introduces unnecessary dependencies.
		template<class T> QList<T> getObjects ();
		template<class T> int countObjects ();

		// *** Selection frontends
		virtual QList<Flight> getPreparedFlights ();
		virtual QList<Flight> getFlightsDate (QDate date);

		// *** Value lists
		virtual QStringList listLocations ();
		virtual QStringList listAccountingNotes ();
		virtual QStringList listClubs ();
		virtual QStringList listPlaneTypes ();

		// *** Additional properties
		template<class T> bool objectUsed (dbId id);

	public slots:
		void cancelConnection ();

	signals:
		void dbEvent (DbEvent event);

	protected:
		void emitDbEvent (DbEvent event);

	private:
		Interface &interface;
};

#endif
