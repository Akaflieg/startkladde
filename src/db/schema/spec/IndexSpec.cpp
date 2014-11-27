#include "IndexSpec.h"

#include <QStringList>

#include "src/i18n/notr.h"

IndexSpec::IndexSpec (const QString &table, const QString &name, const QString &columns):
	table (table), name (name), columns (columns)
{
}

//static IndexSpec IndexSpec::singleColumn (const QString &table, const QString &column)
//{
//	return IndexSpec (table, column+"_index", column);
//}

IndexSpec::~IndexSpec ()
{
}

QString IndexSpec::createClause () const
{
	return qnotr ("INDEX %1 (%2)").arg (name, columns);
}

QString IndexSpec::createClause (const QList<IndexSpec> &list)
{
	QStringList createClauses;

	foreach (const IndexSpec &indexSpec, list)
		createClauses.append (indexSpec.createClause ());

	return createClauses.join (notr (", "));
}
