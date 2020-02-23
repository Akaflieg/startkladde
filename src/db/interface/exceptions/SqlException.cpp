#include "SqlException.h"

#include "src/io/AnsiColors.h"
#include "src/i18n/notr.h"

SqlException::SqlException (const QSqlError &error):
	error (error)
{
}

SqlException::~SqlException ()
{
}

QString SqlException::makeString (const QString &message) const
{
	return qnotr (
		"%1\n"
		"    Number/type   : %2/%3\n"
		"    Database error: %4\n"
		"    Driver error  : %5")
		.arg (message)
        .arg (error.nativeErrorCode ()).arg (error.type ())
		.arg (error.databaseText ())
		.arg (error.driverText ());
}

QString SqlException::makeColorizedString (const QString &message) const
{
	AnsiColors c;

	return qnotr (
		"%3\n"
		"    Number/type   : %1%4%2/%5\n"
		"    Database error: %1%6%2\n"
		"    Driver error  : %7")
		.arg (c.red ()).arg (c.reset ())
		.arg (message)
        .arg (error.nativeErrorCode ()).arg (error.type ())
		.arg (error.databaseText ())
		.arg (error.driverText ());
}
