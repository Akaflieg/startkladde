/*
 * - listing columns: select * from table where false (or describe table/show
 *   columns from table, but unclear column order)
 *
 */
#include "Interface.h"

#include <iostream>

#include <QStringList>
#include <QVariant>
#include <QCryptographicHash>
#include <QString>
#include <QSqlError>

// TODO should go to DefaultInterface/MySQLInterface
#include <errmsg.h>
#include <mysqld_error.h>

#include "src/db/result/Result.h"
#include "src/db/Query.h"
#include "src/util/qString.h"
#include "src/db/schema/spec/ColumnSpec.h"
#include "src/db/schema/spec/IndexSpec.h"
#include "src/db/interface/exceptions/QueryFailedException.h" // TODO remove
#include "src/i18n/notr.h"

// ******************
// ** Construction **
// ******************

Interface::Interface (const DatabaseInfo &dbInfo):
	AbstractInterface (dbInfo)
{
}

Interface::~Interface ()
{
}


// ****************
// ** Data types **
// ****************

// Note: these values are used in migrations. If they are changed, the
// migrations should be updated to use the same values as before.
QString Interface::dataTypeBinary    () { return notr ("blob")            ; }
QString Interface::dataTypeBoolean   () { return notr ("tinyint(1)")      ; }
QString Interface::dataTypeDate      () { return notr ("date")            ; }
QString Interface::dataTypeDatetime  () { return notr ("datetime")        ; }
QString Interface::dataTypeDecimal   () { return notr ("decimal")         ; }
QString Interface::dataTypeFloat     () { return notr ("float")           ; }
QString Interface::dataTypeInteger   () { return notr ("int(11)")         ; }
QString Interface::dataTypeLong      () { return notr ("bigint")          ; }
QString Interface::dataTypeString    () { return notr ("varchar(255)")    ; }
QString Interface::dataTypeString16  () { return notr ("varchar(16)")     ; }
QString Interface::dataTypeText      () { return notr ("text")            ; }
QString Interface::dataTypeTime      () { return notr ("time")            ; }
QString Interface::dataTypeTimestamp () { return notr ("datetime")        ; }
QString Interface::dataTypeCharacter () { return notr ("varchar(1)")      ; } // Non-Rails
QString Interface::dataTypeId        () { return dataTypeInteger (); }


// *********************
// ** User management **
// *********************

void Interface::grantAll (const QString &database, const QString &username, const QString &password)
{
	Query query=Query (notr ("GRANT ALL ON %1.* TO '%2'@'%'"))
		.arg (database).arg (username);

	// Client side hashing has the advantage that the password is never
	// transmitted as part of a query, so we can display the query to the
	// user in a log.
	if (!password.isEmpty())
		query+=Query (notr ("IDENTIFIED BY PASSWORD '%1'")).arg (mysqlPasswordHash (password));

	executeQuery (query);

	// For MySQL, % does not include localhost.
	// TODO grantQuery
	query=Query (notr ("GRANT ALL ON %1.* TO '%2'@'localhost'"))
		.arg (database).arg (username);
	if (!password.isEmpty())
		query+=Query (notr ("IDENTIFIED BY PASSWORD '%1'")).arg (mysqlPasswordHash (password));
	executeQuery (query);
}

// *************************
// ** Schema manipulation **
// *************************

void Interface::createDatabase (const QString &name, bool skipIfExists)
{
	std::cout << qnotr ("Creating database %1%2")
		.arg (name, skipIfExists?notr (" if it does not exist"):"")
		<< std::endl;

	executeQuery (Query (notr ("CREATE DATABASE %1 %2"))
		.arg (skipIfExists?notr ("IF NOT EXISTS"):"").arg (name));
}

/**
 * Creates a table with an ID column
 *
 * A table without columns is not allowed, so this methods creates an ID column
 * (see #idColumn).
 *
 * @param name
 * @param skipIfExists
 * @see #createTable
 */
void Interface::createTable (const QString &name, bool skipIfExists)
{
	createTable (name, QList<ColumnSpec> () << idColumn (), skipIfExists);
}

void Interface::createTable (const QString &name, const QList<ColumnSpec> &columns, bool skipIfExists)
{
	if (skipIfExists)
		std::cout << qnotr ("Creating table %1 if it does not exist").arg (name) << std::endl;
	else
		std::cout << qnotr ("Creating table %1").arg (name) << std::endl;

	// TODO CreateTableQuery
	executeQuery (Query (notr (
		"CREATE TABLE %1 %2 ("
		"%3"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
		))
		.arg (skipIfExists?notr ("IF NOT EXISTS"):"", name, ColumnSpec::createClause (columns)));
}

void Interface::createTable (const QString &name, const QList<ColumnSpec> &columns, const QList<IndexSpec> &indexes, bool skipIfExists)
{
	if (skipIfExists)
		std::cout << qnotr ("Creating table %1 if it does not exist").arg (name) << std::endl;
	else
		std::cout << qnotr ("Creating table %1").arg (name) << std::endl;

	QString createColumnsClause=ColumnSpec::createClause (columns);

	QString createIndexesClause;
	if (!indexes.isEmpty ())
		createIndexesClause=qnotr (", %1").arg (IndexSpec::createClause (indexes));

	// TODO CreateTableQuery
	executeQuery (Query (notr (
		"CREATE TABLE %1 %2 ("
		"%3%4"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
		))
		.arg (skipIfExists?notr ("IF NOT EXISTS"):"", name, createColumnsClause, createIndexesClause));
}



void Interface::createTableLike (const QString &like, const QString &name, bool skipIfExists)
{
	if (skipIfExists)
		std::cout << qnotr ("Creating table %1 like %2 if it does not exist").arg (name, like) << std::endl;
	else
		std::cout << qnotr ("Creating table %1 like %2").arg (name, like) << std::endl;

	// TODO CreateTableLikeQuery
	executeQuery (Query (notr ("CREATE TABLE %1 %2 LIKE %3"))
		.arg (skipIfExists?notr ("IF NOT EXISTS"):"", name, like));
}

void Interface::dropTable (const QString &name)
{
	std::cout << qnotr ("Dropping table %1").arg (name) << std::endl;

	executeQuery (Query (notr ("DROP TABLE %1")).arg (name));
}

void Interface::renameTable (const QString &oldName, const QString &newName)
{
	std::cout << qnotr ("Renaming table %1 to %2").arg (oldName, newName) << std::endl;

	executeQuery (Query (notr ("RENAME TABLE %1 TO %2")).arg (oldName, newName));
}

bool Interface::tableExists ()
{
	return queryHasResult (Query (notr ("SHOW TABLES")));
}

bool Interface::tableExists (const QString &name)
{
	// Using addBindValue does not seem to work here
	return queryHasResult (Query (notr ("SHOW TABLES LIKE '%1'")).arg (name));
}

QStringList Interface::showTables ()
{
	return listStrings (notr ("SHOW TABLES"));
}

void Interface::addColumn (const QString &table, const QString &name, const QString &type, const QString &extraSpecification, bool skipIfExists)
{
	if (skipIfExists && columnExists (table, name))
	{
		std::cout << qnotr ("Skipping existing column %1.%2").arg (table, name) << std::endl;
		return;
	}

	std::cout << qnotr ("Adding column %1.%2").arg (table, name) << std::endl;

	executeQuery (Query (notr ("ALTER TABLE %1 ADD COLUMN %2 %3 %4"))
		.arg (table, name, type, extraSpecification));
}

void Interface::changeColumnType (const QString &table, const QString &name, const QString &type, const QString &extraSpecification)
{
	std::cout << qnotr ("Changing column %1.%2 type to %3")
		.arg (table, name, type) << std::endl;

	executeQuery (Query (notr ("ALTER TABLE %1 MODIFY %2 %3 %4"))
		.arg (table, name, type, extraSpecification));
}

void Interface::dropColumn (const QString &table, const QString &name, bool skipIfNotExists)
{
	if (skipIfNotExists && !columnExists (table, name))
	{
		std::cout << qnotr ("Skipping non-existing column %1.%2")
			.arg (table, name) << std::endl;
		return;
	}

	std::cout << qnotr ("Dropping column %1.%2")
		.arg (table, name) << std::endl;

	executeQuery (Query (notr ("ALTER TABLE %1 DROP COLUMN %2"))
		.arg (table, name));
}

void Interface::renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, const QString &extraSpecification)
{
	std::cout << qnotr ("Renaming column %1.%2 to %3")
		.arg (table, oldName, newName) << std::endl;

	executeQuery (
		Query (notr ("ALTER TABLE %1 CHANGE %2 %3 %4 %5"))
		.arg (table, oldName, newName, type, extraSpecification));
}

bool Interface::columnExists (const QString &table, const QString &name)
{

	return queryHasResult (
		Query (notr ("SHOW COLUMNS FROM %1 LIKE '%2'"))
		.arg (table, name));
}

// TODO: this is backend specific
QList<IndexSpec> Interface::showIndexes (const QString &table)
{
	QList<IndexSpec> indexes;

	Query query=Query (notr ("SHOW INDEXES FROM %1")).arg (table);
	QSharedPointer<Result> result=executeQueryResult (query);

	QSqlRecord record=result->record ();
	// TODO: handle index not found
	int     nameIndex=record.indexOf (notr ("Key_name"));
	int   columnIndex=record.indexOf (notr ("Column_name"));
	int sequenceIndex=record.indexOf (notr ("Seq_in_index"));

	// First, create a nested map:
	// name -> (sequence -> column)
	// Use a QMap rather than a QHash because the values are sorted by key.
	// Note that performance is not an issue.

	// Example:
	// name             sequence    column
	// ---------------------------------------------
	// pilot_id_index   1           pilot_id
	// status_index     1           departed
	// status_index     2           landed
	// status_index     3           towflight_landed

	QMap<QString, QMap <int, QString> > map;

	while (result->next ())
	{
		QString name    =result->value (    nameIndex).toString ();
		QString column  =result->value (  columnIndex).toString ();
		int     sequence=result->value (sequenceIndex).toInt ();

		if (name!=notr ("PRIMARY"))
			map[name][sequence]=column;
	}

	// Second, create the indexes from the map
	foreach (const QString &name, map.keys ())
	{
		QMap <int, QString> sequenceColumnMap=map[name];

		QStringList columnList (sequenceColumnMap.values ());

		indexes.append (IndexSpec (table, name, columnList.join (notr (","))));
	}

	return indexes;
}

void Interface::createIndex (const IndexSpec &index, bool skipIfExists)
{
	if (skipIfExists)
		std::cout << qnotr ("Creating index %1.%2 if it does not exist").arg (index.getTable (), index.getName ()) << std::endl;
	else
		std::cout << qnotr ("Creating index %1.%2").arg (index.getTable (), index.getName ()) << std::endl;

	// TODO: the MySQL specific stuff should not be here
	try
	{
		// TODO createIndexQuery
		executeQuery (qnotr ("CREATE INDEX %1 ON %2 (%3)")
			.arg (index.getName (), index.getTable (), index.getColumns ()));
	}
	catch (QueryFailedException &ex)
	{
		// TODO: better check if the index exists before creating, don't
		// execute the query at all in this case (also, this currently emits
		// an error message)
        int number = extractNativeErrorNumber(ex.error);
        if (skipIfExists && number == ER_DUP_KEYNAME)
			std::cout << qnotr ("Skipping existing index %1.%2").arg (index.getTable (), index.getName ()) << std::endl;
		else
			throw;
	}
}

void Interface::dropIndex (const QString &table, const QString &name, bool skipIfNotExists)
{
	if (skipIfNotExists)
		std::cout << qnotr ("Dropping index %1.%2 if it exists").arg (table, name) << std::endl;
	else
		std::cout << qnotr ("Dropping index %1.%2").arg (table, name) << std::endl;

	try
	{
		executeQuery (qnotr ("DROP INDEX %1 ON %2").arg (name, table));
	}
	catch (QueryFailedException &ex)
	{
        int number = extractNativeErrorNumber(ex.error);
        if (skipIfNotExists && number == ER_CANT_DROP_FIELD_OR_KEY)
			std::cout << qnotr ("Skipping non-existing index %1.%2").arg (table, name) << std::endl;
		else
			throw;
	}
}



ColumnSpec Interface::idColumn ()
{
	return ColumnSpec (notr ("id"), dataTypeId (), notr ("NOT NULL AUTO_INCREMENT PRIMARY KEY"));
}

// *******************************
// ** Generic data manipulation **
// *******************************

void Interface::updateColumnValues (const QString &tableName, const QString &columnName,
	const QVariant &oldValue, const QVariant &newValue)
{
	executeQuery (Query (notr ("UPDATE %1 SET %2=? WHERE %2=?"))
		.arg (tableName, columnName).bind (newValue).bind (oldValue));
}

// TODO template: QList<T> listValues (const Query &query, (method pointer) QVariantMethod)
QStringList Interface::listStrings (const Query &query)
{
	QSharedPointer<Result> result=executeQueryResult (query, true);

	QStringList stringList;

	while (result->next ())
		stringList.append (result->value (0).toString ());

	return stringList;
}

int Interface::countQuery (const Query &query)
{
	QSharedPointer<Result> result=executeQueryResult (query, true);

	result->next ();
	return result->value (0).toInt ();
}


// **********
// ** Misc **
// **********

QString Interface::mysqlPasswordHash (const QString &password)
{
	QByteArray data=password.toUtf8 ();
	data=QCryptographicHash::hash (data, QCryptographicHash::Sha1);
	data=QCryptographicHash::hash (data, QCryptographicHash::Sha1);

	return qnotr ("*%1").arg (QString (data.toHex ()).toUpper ());
}

int Interface::extractNativeErrorNumber(QSqlError& error) {
    bool ok = false;
    int number = error.nativeErrorCode().toInt(&ok);
    if (!ok) {
        number = -1;
    }
    return number;
}
