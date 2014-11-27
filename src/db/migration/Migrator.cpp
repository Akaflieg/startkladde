#include "Migrator.h"

#include <iostream>

#include <QApplication>
#include <QSharedPointer>

#include "src/db/interface/Interface.h"
#include "src/db/migration/MigrationFactory.h"
#include "src/db/schema/CurrentSchema.h"
#include "src/util/qString.h"
#include "src/db/result/Result.h"
#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/i18n/notr.h"

// ***************
// ** Constants **
// ***************

// If this is changed, things will break, like, horribly.
const QString Migrator::migrationsTableName =notr ("schema_migrations");
const QString Migrator::migrationsColumnName=notr ("version");

/**
 * Creates a Migrator for the given Interface
 *
 * @param interface the Interface to access
 */
Migrator::Migrator (Interface &interface):
	interface (interface)
{
}

Migrator::~Migrator ()
{
}


// ****************
// ** Migrations **
// ****************

void Migrator::runMigration (quint64 version, Migration::Direction direction, OperationMonitorInterface monitor)
{
	Migration *migration=NULL;
	try
	{
		migration   =MigrationFactory::instance ().createMigration (interface, version);
		QString name=MigrationFactory::instance ().migrationName (version);

		switch (direction)
		{
			case Migration::dirUp:
				std::cout << notr ("== Applying: ") << name << notr (" ") << QString (79-14-name.length (), '=') << std::endl;
				monitor.status (qApp->translate ("Migrator", "Applying migration: %1").arg (name));
				migration->up ();
				addMigration (version);
				break;
			case Migration::dirDown:
				std::cout << notr ("== Reverting: ") << name << notr (" ") << QString (79-15-name.length (), '=') << std::endl;
				monitor.status (qApp->translate ("Migrator", "Reverting migration: %1").arg (name));
				migration->down ();
				removeMigration (version);
				break;
		}

		std::cout << notr ("== Version is now ") << currentVersion () << notr (" ") << QString (79-19-14, '=') << std::endl << std::endl;

		delete migration;
	}
	catch (...)
	{
		delete migration;
		throw;
	}
}

/** Migrates one step up */
void Migrator::up ()
{
	quint64 version=nextMigration ();

	if (version==0)
	{
		std::cout << notr ("Already current") << std::endl;
		return;
	}

	runMigration (version, Migration::dirUp);
}

/** Migrates one step down */
void Migrator::down ()
{
	quint64 version=currentVersion ();

	if (version==0) return;

	runMigration (currentVersion (), Migration::dirDown);
}

/** Migrates to the latest version (runs pending migrations) */
void Migrator::migrate (OperationMonitorInterface monitor)
{
	QList<quint64> versions=pendingMigrations ();

	int progress=0, maxProgress=pendingMigrations ().size ();

	monitor.progress (progress, maxProgress);
	foreach (quint64 version, versions)
	{
		runMigration (version, Migration::dirUp, monitor);
		++progress;
		monitor.progress (progress, maxProgress);
	}
}


// ************
// ** Schema **
// ************

void Migrator::loadSchema (OperationMonitorInterface monitor)
{
	std::cout << notr ("== Loading schema =============================================================") << std::endl;

	CurrentSchema schema (interface);

	// Create the migrations table before loading the schema so we can
	// distinguish "load canceled" (migrations table and some tables exists)
	// from "old database" (migrations table does not exist, some tables
	// exist).
	monitor.status (qApp->translate ("Migrator", "Creating migrations table"));
	createMigrationsTable ();
	// Clear the table because if the migrations table is non-empty, but no
	// other tables exist, loadScheme will be called. If the loading is
	// canceled, the resulting state could not be distinguished from a
	// regular (current or non-current) database, but the scheme would be
	// wrong, so using or migrating the database would fail.
	clearMigrationsTable ();

	monitor.status (qApp->translate ("Migrator", "Loading schema"));
	schema.up (monitor);

	monitor.status (qApp->translate ("Migrator", "Saving version"));
	assumeMigrated (schema.getVersions ());

	std::cout << notr ("== Version is now ") << currentVersion () << notr (" ") << QString (79-19-14, '=') << std::endl << std::endl;
}

void Migrator::drop ()
{
	// TODO create and use db method
	QString databaseName=interface.getInfo ().database;
	interface.executeQuery (qnotr ("DROP DATABASE %1").arg (databaseName));
}

void Migrator::create ()
{
	// TODO use db method
	QString databaseName=interface.getInfo ().database;
	interface.executeQuery (qnotr ("CREATE DATABASE %1").arg (databaseName));
}

void Migrator::clear ()
{
	drop ();
	create ();
}

void Migrator::reset ()
{
	drop ();
	create ();
	loadSchema ();
}


// *************
// ** Version **
// *************

/**
 * @param currentVersion set to the current schema verison if the result is actionMigrate
 * @param monitor
 * @return
 */
Migrator::Action Migrator::getRequiredAction (quint64 *currentVersion, int *numPendingMigrations, OperationMonitorInterface monitor)
{
	// # | Migrations table | Other tables || Empty | Action  | Comments
	// --+------------------+--------------++-------+---------+---------------------------------------
	// 1 | not existing     | no           || yes   | load    | Regular creating
	// 2 | not existing     | yes          || no    | migrate | Old (pre-migration) database
	// 3 | empty            | no           || no    | load    | Load canceled (before creating tables)
	// 4 | empty            | yes          || no    | load    | Load canceled (while creating tables)
	// 5 | not current      | no           || no    | load    | Error: non-current, but tables missing
	// 6 | not current      | yes          || no    | migrate | Regular migration
	// 7 | current          | no           || no    | load    | Error: current, but tables missing
	// 8 | current          | yes          || no    | none    | Regular current
	//
	//                          Other tables:
	//                          no    yes
	//                         ,-------------
	//            not existing |load  migrate
	// Migrations empty        |load  load
	// table:     not current  |load  migrate
	//            current      |load  none

	monitor.status  (qApp->translate ("Migrator", "Checking database"));
	QStringList tables=interface.showTables ();

	bool migrationsTableExists=tables.contains (migrationsTableName);

	tables.removeAll (migrationsTableName);
	bool otherTablesExist=!tables.empty ();

	// # 1,3,5,6 - no other tables exist
	if (!otherTablesExist) return actionLoad;

	// #2 - old database
	if (!migrationsTableExists)
	{
		if (currentVersion) *currentVersion=0;
		if (numPendingMigrations) *numPendingMigrations=
			MigrationFactory::instance ().availableVersions ().size ();
		return actionMigrate;
	}

	// #4 - load canceled
	monitor.status (qApp->translate ("Migrator", "Checking database version"));
	quint64 current=this->currentVersion ();
	if (current==0) return actionLoad;

	// #8 - current
	quint64 latest=this->latestVersion ();
	if (current==latest) return actionNone;

	// #6 - not current
	if (currentVersion) *currentVersion=current;
	if (numPendingMigrations) *numPendingMigrations=pendingMigrations ().size ();
	return actionMigrate;
}



// ***********************
// ** Migration listing **
// ***********************

QList<quint64> Migrator::pendingMigrations ()
{
	QList<quint64> availableMigrations=
		MigrationFactory::instance ().availableVersions ();
	QList<quint64> appliedMigrations=this->appliedMigrations ();

	QList<quint64> pending;
	foreach (quint64 version, availableMigrations)
		if (!appliedMigrations.contains (version))
			pending << version;

	return pending;
}

quint64 Migrator::nextMigration ()
{
	QList<quint64> availableMigrations=
		MigrationFactory::instance ().availableVersions ();
	QList<quint64> appliedMigrations=this->appliedMigrations ();

	foreach (quint64 version, availableMigrations)
		if (!appliedMigrations.contains (version))
			return version;

	return 0;
}

quint64 Migrator::latestVersion ()
{
	return MigrationFactory::instance ().latestVersion ();
}

bool Migrator::isCurrent (OperationMonitorInterface monitor)
{
	monitor.status (qApp->translate ("Migrator", "Checking database version"));

	// Use nextMigration, not currentVersion, so it works with gaps
	return nextMigration ()==0;
}

bool Migrator::isEmpty (OperationMonitorInterface monitor)
{
	monitor.status (qApp->translate ("Migrator", "Checking database"));
	return !interface.tableExists ();
}


// **********************
// ** Migrations table **
// **********************

/**
 * Determines the version of the database (the version of the lates migration)
 *
 * @return the version, or an 0 if the version table does not exist or is empty
 */
quint64 Migrator::currentVersion ()
{
	if (!interface.tableExists (migrationsTableName)) return 0;

	Query query=Query (notr ("SELECT %2 FROM %1 ORDER BY %2 DESC LIMIT 1"))
		.arg (migrationsTableName, migrationsColumnName);

	QSharedPointer<Result> result=interface.executeQueryResult (query);

	// TODO getValue query
	if (result->next ())
		return result->value (0).toLongLong ();
	else
		return 0;
}

void Migrator::createMigrationsTable ()
{
	// TODO use interface.createTable
	interface.executeQuery (
		qnotr (
			"CREATE TABLE IF NOT EXISTS %1 (%2 VARCHAR(255) NOT NULL PRIMARY KEY)"
			" ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
		).arg (migrationsTableName, migrationsColumnName)
	);
}

void Migrator::clearMigrationsTable ()
{
	interface.executeQuery (Query (notr ("DELETE FROM %1")).arg (migrationsTableName));
}

bool Migrator::hasMigration (quint64 version)
{
	return interface.queryHasResult (
		Query (notr ("SELECT %2 FROM %1 WHERE %2=?"))
		.arg (migrationsTableName, migrationsColumnName)
		.bind (version)
		);
}

void Migrator::addMigration (quint64 version)
{
	// If the migrations table does not exist, create it
	if (!interface.tableExists (migrationsTableName)) createMigrationsTable ();

	// If the migration name is already present in the migrations table, return
	if (hasMigration (version)) return;

	// Add the migration name to the migrations table
	interface.executeQuery (
		Query (notr ("INSERT INTO %1 (%2) VALUES (?)"))
		.arg (migrationsTableName, migrationsColumnName)
		.bind (version)
	);
}

void Migrator::removeMigration (quint64 version)
{
	// If the migrations table does not exist, return
	if (!interface.tableExists (migrationsTableName)) return;

	// Remove the migration name from the migrations table
	interface.executeQuery (
		Query (notr ("DELETE FROM %1 where %2=?"))
			.arg (migrationsTableName, migrationsColumnName)
			.bind (version)
	);
}

QList<quint64> Migrator::appliedMigrations ()
{
	QList<quint64> migrations;

	if (!interface.tableExists (migrationsTableName)) return migrations;

	Query query=Query::selectDistinctColumns (
		migrationsTableName, migrationsColumnName);

	QSharedPointer<Result> result=interface.executeQueryResult (query);

	// TODO listValues method
	while (result->next ())
		migrations.append (result->value (0).toLongLong ());

	return migrations;
}

/**
 * Assumes that the migrations table exists and is empty
 *
 * @param versions
 */
void Migrator::assumeMigrated (QList<quint64> versions)
{
	// Add all migrations

	// Add all migrations in one single query because the database would be
	// inconsistent if the process was interrupted with only some migrations
	// inserted.

	// Create a list of placeholders: "(?),(?),...,(?)"
	QStringList placeholders;
	for (int i=0; i<versions.size (); ++i)
		placeholders << notr ("(?)");

	Query query=Query (notr ("INSERT INTO %1 (%2) VALUES %3"))
		.arg (migrationsTableName, migrationsColumnName, placeholders.join (notr (",")));

	foreach (quint64 version, versions)
		query.bind (version);

	interface.executeQuery (query);
}
