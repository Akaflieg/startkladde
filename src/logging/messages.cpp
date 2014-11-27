#include "messages.h"

#include <iostream>

#include <QDateTime>
#include <QString>

#include "src/io/AnsiColors.h"
#include "src/util/qString.h"
#include "src/i18n/notr.h"

QString timestamp ()
{
	return QDateTime::currentDateTime ().toString (Qt::ISODate);
}

void log_message (const QString &message)
	/*
	 * Writes a message, prefixed with date and time, to the log, whatever the
	 * log is.
	 * At the moment, it's stdout.
	 * Parameters:
	 *   - message: The message to write.
	 */
{
	AnsiColors c;
	std::cout << qnotr ("%1%2: %3%4")
		.arg (c.yellow (), timestamp (), message, c.reset ()) << std::endl;
}

void log_error (const QString &message)
	/*
	 * Writes an error message, prefixed with date and time, to the log,
	 * whatever the log is.
	 * At the moment, it's stderr.
	 * Parameters:
	 *   - message: The message to write.
	 */
{
	AnsiColors c;
	std::cerr << qnotr ("%1%2: Error: %3%4")
		.arg (c.red (), timestamp (), message, c.reset ()) << std::endl;
}
