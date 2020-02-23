/*
 * Interface.h
 *
 *  Created on: 25.02.2010
 *      Author: Martin Herrmann
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "src/db/interface/AbstractInterface.h"

class QVariant;

class ColumnSpec;
class IndexSpec;

/**
 * An abstract class that adds some functionality (like schema
 * manipulation and generic data manipulation) to AbstractInterface.
 *
 * Interface implementations should inherit from this class rather than
 * from AbstractInterface.
 *
 * For further information about the Interface class hierarchy see,
 * doc/interal/databaseArchitecture.txt
 *
 * Note: the methods of this class are used by the migrations. If any if the
 * methods are changed, the migrations have to be updated to retain their
 * original functionality.
 */
class Interface: public AbstractInterface
{
	public:
		// *** Construction
		Interface (const DatabaseInfo &dbInfo);
		virtual ~Interface ();

		// *** Data types
		// Data type names like in Rails (use for sk_web) (for MySQL)
		// Not implemented as static constants in order to avoid the static
		// initialization order fiasco. Also, we'll probably want to introduce
		// variable length strings and data types depending on the backend.
		static QString dataTypeBinary    ();
		static QString dataTypeBoolean   ();
		static QString dataTypeDate      ();
		static QString dataTypeDatetime  ();
		static QString dataTypeDecimal   ();
		static QString dataTypeFloat     ();
		static QString dataTypeInteger   ();
		static QString dataTypeString    ();
		static QString dataTypeString16  ();
		static QString dataTypeText      ();
		static QString dataTypeTime      ();
		static QString dataTypeTimestamp ();
		static QString dataTypeCharacter (); // Non-Rails
		static QString dataTypeId        ();

		// *** User management
		void grantAll (const QString &database, const QString &username, const QString &password="");

		// *** Schema manipulation
		void createDatabase (const QString &name, bool skipIfExists=false);
		void createTable (const QString &name, bool skipIfExists=false);
		void createTable (const QString &name, const QList<ColumnSpec> &columns, bool skipIfExists=false);
		void createTable (const QString &name, const QList<ColumnSpec> &columns, const QList<IndexSpec> &indexes, bool skipIfExists=false);
		void createTableLike (const QString &like, const QString &name, bool skipIfExists=false);
		void dropTable (const QString &name);
		void renameTable (const QString &oldName, const QString &newName);
		bool tableExists ();
		bool tableExists (const QString &name);
		QStringList showTables ();
		ColumnSpec idColumn ();

		void addColumn (const QString &table, const QString &name, const QString &type, const QString &extraSpecification="", bool skipIfExists=false);
		void changeColumnType (const QString &table, const QString &name, const QString &type, const QString &extraSpecification="");
		void dropColumn (const QString &table, const QString &name, bool skipIfNotExists=false);
		void renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, const QString &extraSpecification="");
		bool columnExists (const QString &table, const QString &name);
		QList<IndexSpec> showIndexes (const QString &table);
		void createIndex (const IndexSpec &index, bool skipIfExists=false);
		void dropIndex (const QString &table, const QString &name, bool skipIfNotExists=false);


		// *** Generic data manipulation
		void updateColumnValues (const QString &tableName, const QString &columnName, const QVariant &oldValue, const QVariant &newValue);
		QStringList listStrings (const Query &query);
		int countQuery (const Query &query);

		// *** Misc
		QString mysqlPasswordHash (const QString &password);
        static int extractNativeErrorNumber(QSqlError& error);
};

#endif
