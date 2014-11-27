#ifndef MIGRATIONFACTORY_H_
#define MIGRATIONFACTORY_H_

#include <QStringList>
#include <QMap>

#include "src/util/qString.h"

class MigrationBuilder;
class Interface;
class Migration;

/**
 * Registers a migration by creating an instance of the corresponding Registration class.
 * Use this macro in the cpp file of the migration implementation.
 */
#define REGISTER_MIGRATION(version, name) \
	MigrationFactory::Registration registration_ ## version ( \
		new MigrationBuilderImplementation<Migration_ ## version ## _ ## name> (\
			version ## ll, #name \
		) \
	);

/**
 * Determines available migrations, creates migrations and determines
 * migration information.
 *
 * See factoryPatterns.txt
 */
class MigrationFactory
{
	public:
		// Use via the REGISTER_MIGRATION macro.
		class Registration
		{
			public:
				Registration (MigrationBuilder *builder);
		};

		class NoSuchMigrationException: public std::exception
		{
			public:
				NoSuchMigrationException (quint64 version): version (version) {}
				quint64 version;
		};

		static MigrationFactory &instance ();
		virtual ~MigrationFactory ();

		QList<quint64> availableVersions () const;
		quint64 latestVersion () const;

		Migration *createMigration (Interface &interface, quint64 version) const;
		QString migrationName (quint64 version) const;

	private:
		// Singleton
		MigrationFactory ();
		static MigrationFactory *instance_;

		// Migration list
		QMap<quint64, MigrationBuilder *> builders; // QMap: sorted by key
};

#endif
