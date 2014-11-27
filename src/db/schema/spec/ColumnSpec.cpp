#include "ColumnSpec.h"

#include <QList>
#include <QStringList>

#include "src/i18n/notr.h"

ColumnSpec::ColumnSpec (const QString &name, const QString &type, const QString &extra):
	name (name), type (type), extra (extra)
{
}

ColumnSpec::~ColumnSpec ()
{
}

QString ColumnSpec::createClause () const
{
	if (extra.isEmpty ())
		return qnotr ("%1 %2").arg (name, type);
	else
		return qnotr ("%1 %2 %3").arg (name, type, extra);
}

QString ColumnSpec::createClause (const QList<ColumnSpec> &list)
{
	QStringList createClauses;

	foreach (const ColumnSpec &columnSpec, list)
		createClauses.append (columnSpec.createClause ());

	return createClauses.join (notr (", "));
}
