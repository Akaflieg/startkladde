#include "TransactionFailedException.h"

#include <cassert>

#include "src/io/AnsiColors.h"
#include "src/i18n/notr.h"

TransactionFailedException::TransactionFailedException (const QSqlError &error,
	AbstractInterface::TransactionStatement statement):
	SqlException (error), statement (statement)
{
}

TransactionFailedException *TransactionFailedException::clone () const
{
	return new TransactionFailedException (error, statement);
}

void TransactionFailedException::rethrow () const
{
	throw TransactionFailedException (error, statement);
}

QString TransactionFailedException::toString () const
{
	return makeString (qnotr (
		"Transaction failed:\n"
		"    Statement     : %1\n")
		.arg (AbstractInterface::transactionStatementString (statement))
		);
}

QString TransactionFailedException::colorizedString () const
{
	AnsiColors c;

	return makeColorizedString (qnotr (
		"%1Transaction failed%2:\n"
		"    Statement     : %3")
		.arg (c.red ()).arg (c.reset ())
		.arg (AbstractInterface::transactionStatementString (statement))
		);
}
