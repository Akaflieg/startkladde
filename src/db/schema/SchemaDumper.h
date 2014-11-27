#ifndef SCHEMADUMPER_H_
#define SCHEMADUMPER_H_

#include <QString>

class Interface;
class IndexSpec;

class SchemaDumper
{
	public:
		SchemaDumper (Interface &interface);
		virtual ~SchemaDumper ();

		QString dumpSchema ();
		void dumpSchemaToFile (const QString &filename);

	protected:
		Interface &interface;

		void dumpTables   (QStringList &output);
		void dumpTable    (QStringList &output, const QString &name);
		void dumpColumns  (QStringList &output, const QString &table);
		void dumpColumn   (QStringList &output, const QString &name, const QString &type, const QString &null, const QString &key, const QString &extra);
		void dumpIndexes  (QStringList &output, const QString &table);
		void dumpIndex    (QStringList &output, const IndexSpec &index);
		void dumpVersions (QStringList &output);
};

#endif
