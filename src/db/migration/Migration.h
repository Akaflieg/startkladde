#ifndef MIGRATION_H_
#define MIGRATION_H_

#include <QString>
#include <QVariant>
#include <QSharedPointer>

#include "src/db/Query.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"
#include "src/db/schema/spec/ColumnSpec.h"
#include "src/db/schema/spec/IndexSpec.h"
#include "src/db/migration/MigrationBuilder.h"
#include "src/db/migration/MigrationFactory.h" // Required for REGISTER_MIGRATION

template<class T> class QList;

class Result;
class Interface;

/**
 * A new migration is created by creating the appropriate class in
 * src/db/migrations/. The migration is automatically included into the build
 * and recognized by MigrationFactory.
 *
 * A new migration class can be created automatically by using the script
 * script/generate_migration.rb (see documentation).
 *
 * Migrations are supposed to be called only through Migrator, which keeps track
 * of which migrations have already been applied (see documentation).
 *
 * Note that we forward a lot of Interface methods (and constants) to
 * Interface instead of having Migration implementations access the
 * Interface directly.
 * This is done in order to avoid a dependency of the individual migrations on
 * Interface, which would necessitate recompiling all migrations when
 * the database interface changes, even if the migrations are not affected by
 * the change.
 *
 * Another potential use of this is that this can be used to change the
 * behavior of the database methods invoked by the migrations, for example to
 * add logging or accumulate changes before executing them.
 *
 * Notes for implementations:
 *   - do not include Interface.h. Use the forwarder methods provided
 *     by this class instead. Add forwarder methods if required.
 *
 * Note that the monitor is not required for cancelation, since the only long
 * operations the migrations do are queries, and queries can be canceled by
 * using the interface's cancelConnection method. We also don't do progress
 * reporting (except currentSchema) because that would only make sense with
 * suboperations, which are not implemented, so we don't use a monitor at all.
 */
class Migration
{
	public:
		// *** Types
		enum Direction { dirUp, dirDown };

		// *** Constants

		// Not implemented as static constants to avoid the static
		// initialization order fiasco.
    	static QString dataTypeBinary    ();
    	static QString dataTypeBoolean   ();
    	static QString dataTypeDate      ();
    	static QString dataTypeDatetime  ();
    	static QString dataTypeDecimal   ();
    	static QString dataTypeFloat     ();
    	static QString dataTypeInteger   ();
    	static QString dataTypeString    ();
    	static QString dataTypeString16  (); // Non-rails
    	static QString dataTypeText      ();
    	static QString dataTypeTime      ();
    	static QString dataTypeTimestamp ();
    	static QString dataTypeCharacter (); // Non-Rails
    	static QString dataTypeId        ();


		Migration (Interface &interface);
		virtual ~Migration ();

		virtual void up ()=0;
		virtual void down ()=0;

	protected:
		void transaction ();
		void commit ();
		void rollback ();

		void executeQuery (const Query &query);
		QSharedPointer<Result> executeQueryResult (const Query &query, bool forwardOnly=true);

		void createTable (const QString &name, bool skipIfExists=true);
		void createTable (const QString &name, const QList<ColumnSpec> &columns, bool skipIfExists=true);
		void createTable (const QString &name, const QList<ColumnSpec> &columns, const QList<IndexSpec> &indexes, bool skipIfExists=true);
		void createTableLike (const QString &like, const QString &name, bool skipIfExists=true);
		void dropTable (const QString &name);
		void renameTable (const QString &oldName, const QString &newName);

		void addColumn (const QString &table, const QString &name, const QString &type, const QString &extraSpecification="", bool skipIfExists=true);
		void changeColumnType (const QString &table, const QString &name, const QString &type, const QString &extraSpecification="");
		void dropColumn (const QString &table, const QString &name, bool skipIfNotExists=true);
		void renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, const QString &extraSpecification="");
		void createIndex (const IndexSpec &index, bool skipIfExists=true);
		void dropIndex (const QString &table, const QString &name, bool skipIfNotExists=true);

		ColumnSpec idColumn ();

		void updateColumnValues (const QString &tableName, const QString &columnName, const QVariant &oldValue, const QVariant &newValue);

	private:
		Interface &interface;
};

#endif
