#include "ConnectionFailedException.h"

#include "src/io/AnsiColors.h"
#include "src/i18n/notr.h"

ConnectionFailedException::ConnectionFailedException (const QSqlError &error):
	SqlException (error)
{
}

ConnectionFailedException *ConnectionFailedException::clone () const
{
	return new ConnectionFailedException (error);
}

void ConnectionFailedException::rethrow () const
{
	throw ConnectionFailedException (error);
}

QString ConnectionFailedException::toString () const
{
	return makeString (qnotr ("Connection failed:"));
}

QString ConnectionFailedException::colorizedString () const
{
	AnsiColors c;

	return makeColorizedString (qnotr (
		"%1Connection failed%2:")
		.arg (c.red ()).arg (c.reset ())
		);
}
