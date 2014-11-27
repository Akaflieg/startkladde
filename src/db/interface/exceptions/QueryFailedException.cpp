#include "QueryFailedException.h"

#include <cassert>

#include "src/io/AnsiColors.h"
#include "src/i18n/notr.h"

QueryFailedException::QueryFailedException (const QSqlError &error, const Query &query,
	QueryFailedException::Phase phase):
	SqlException (error), query (query), phase (phase)
{
}

QueryFailedException QueryFailedException::prepare (const QSqlError &error, const Query &query)
{
	return QueryFailedException (error, query, phasePrepare);
}

QueryFailedException QueryFailedException::execute (const QSqlError &error, const Query &query)
{
	return QueryFailedException (error, query, phaseExecute);
}

QueryFailedException *QueryFailedException::clone () const
{
	return new QueryFailedException (error, query, phase);
}

void QueryFailedException::rethrow () const
{
	throw QueryFailedException (error, query, phase);
}

QString QueryFailedException::toString () const
{
	return makeString (qnotr (
		"Query failed during %1:\n"
		"    Query         : %2\n")
		.arg (phaseString (phase))
		.arg (query.toString ())
		);
}

QString QueryFailedException::colorizedString () const
{
	AnsiColors c;

	return makeColorizedString (qnotr (
		"%1Query failed%2 during %3:\n"
		"    Query         : %4")
		.arg (c.red ()).arg (c.reset ())
		.arg (phaseString (phase))
		.arg (query.colorizedString ())
		);
}

QString QueryFailedException::phaseString (QueryFailedException::Phase phase)
{
	switch (phase)
	{
		case phasePrepare: return notr ("prepare");
		case phaseExecute: return notr ("execute");
		// no default
	}

	assert (!notr ("Unhandled phase"));
	return notr ("?");
}
