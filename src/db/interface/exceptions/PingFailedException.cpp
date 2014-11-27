#include "PingFailedException.h"

#include "src/io/AnsiColors.h"
#include "src/i18n/notr.h"

PingFailedException::PingFailedException (const QSqlError &error):
	SqlException (error)
{
}

PingFailedException *PingFailedException::clone () const
{
	return new PingFailedException (error);
}

void PingFailedException::rethrow () const
{
	throw PingFailedException (error);
}

QString PingFailedException::toString () const
{
	return makeString (qnotr ("Ping failed:"));
}

QString PingFailedException::colorizedString () const
{
	AnsiColors c;

	return makeColorizedString (qnotr (
		"%1Ping failed%2:")
		.arg (c.red ()).arg (c.reset ())
		);
}
