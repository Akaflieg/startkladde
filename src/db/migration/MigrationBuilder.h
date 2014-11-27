#ifndef MIGRATIONBUILDER_H_
#define MIGRATIONBUILDER_H_

#include "src/db/migration/MigrationFactory.h"

class Migration;
class Interface;


/**
 * See factoryPatterns.txt
 */
class MigrationBuilder
{
	public:
		virtual ~MigrationBuilder () {}

		virtual quint64 getVersion ()=0;
		virtual QString getName ()=0;

		virtual Migration *build (Interface &interface)=0;
};

template<class T> class MigrationBuilderImplementation: public MigrationBuilder
{
	public:
		MigrationBuilderImplementation (quint64 version, const QString &name):
			version (version), name (name)
		{
		}

		quint64 getVersion ()
		{
			return version;
		}

		QString getName ()
		{
			return name;
		}

		/**
		 * Create a new instance of the migration. The caller takes ownership
		 * of the returned object.
		 */
		T *build (Interface &interface)
		{
			return new T (interface);
		}

	private:
		quint64 version;
		QString name;
};

#endif
