#include "MigrationFactory.h"

#include "src/i18n/notr.h"
#include "src/db/migration/MigrationBuilder.h"

// ****************************
// ** Construction/Singleton **
// ****************************

MigrationFactory *MigrationFactory::instance_;

MigrationFactory::MigrationFactory ()
{
}

MigrationFactory &MigrationFactory::instance ()
{
	if (!instance_)
		instance_=new MigrationFactory ();

	return *instance_;
}

MigrationFactory::~MigrationFactory ()
{
	// Delete the builders, they are owned by the factory. This is never going
	// to happen, though, because the factory is a singleton and is never
	// deleted.
	foreach (MigrationBuilder *builder, builders.values ())
		delete builder;
}


// **********************
// ** Migration access **
// **********************

/**
 * Lists the versions of all available migrations, sorted in ascending order.
 *
 * @return a QList of migration versions
 */
QList<quint64> MigrationFactory::availableVersions () const
{
	// QMap returns the keys in sorted order
	return builders.keys ();
}

/**
 * Determines the latest available migration. This is the last entry of the
 * list returned by #availableVersions, unless no migrations exist.
 *
 * @return the latest available migration, or 0 if no migrations exist
 */
quint64 MigrationFactory::latestVersion () const
{
	QList<quint64> versions=availableVersions ();

	if (versions.empty ())
		return 0;
	else
		return versions.last ();
}

/**
 * Creates a new Migration instance of the given version.
 *
 * The caller takes ownership of the Migration.
 *
 * @param database the database to create the migration for
 * @param version the version of the migration to generate. Must have been
 *                obtained from the same MigrationFactory instance.
 * @return a newly allocated instance of Migration for the given version
 * @throw NoSuchMigrationException if there is no migration with the given
 *        version
 */

Migration *MigrationFactory::createMigration (Interface &interface, const quint64 version) const
{
	MigrationBuilder *builder=builders.value (version, NULL);
	if (builder)
		return builder->build (interface);
	else
		throw NoSuchMigrationException (version);

	throw NoSuchMigrationException (version);
}

/**
 * Determines the name of a given migration version.
 *
 * @param version the version of the migration to determine the name of. Must
 *                have been obtained from the same MigrationFactory instance.
 * @return the name of the migration with the given version
 * @throw NoSuchMigrationException if there is no migration with the given
 *        version
 */
QString MigrationFactory::migrationName (quint64 version) const
{
	MigrationBuilder *builder=builders.value (version, NULL);
	if (builder)
		return builder->getName ();
	else
		throw NoSuchMigrationException (version);
}


// ********************************************
// ** MigrationFactory::Registration methods **
// ********************************************

MigrationFactory::Registration::Registration (MigrationBuilder *builder)
{
	MigrationFactory::instance ().builders.insert (builder->getVersion (), builder);
}
