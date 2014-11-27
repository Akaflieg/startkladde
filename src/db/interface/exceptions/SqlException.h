/*
 * SqlException.h
 *
 *  Created on: 01.03.2010
 *      Author: Martin Herrmann
 */

#ifndef SQLEXCEPTION_H_
#define SQLEXCEPTION_H_

#include <QSqlError>
#include <QString>

#include "src/StorableException.h"

class SqlException: public StorableException
{
	public:
		SqlException (const QSqlError &error);
		virtual ~SqlException ();

		virtual QString toString () const=0;
		virtual QString colorizedString () const=0;

		QSqlError error;

	protected:
		virtual QString makeString (const QString &message) const;
		virtual QString makeColorizedString (const QString &message) const;
};

#endif
