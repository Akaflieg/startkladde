/*
 * ConnectionFailedException.h
 *
 *  Created on: 01.03.2010
 *      Author: Martin Herrmann
 */

#ifndef CONNECTIONFAILEDEXCEPTION_H_
#define CONNECTIONFAILEDEXCEPTION_H_

#include "src/db/interface/exceptions/SqlException.h"

class ConnectionFailedException: public SqlException
{
	public:
		ConnectionFailedException (const QSqlError &error);

		virtual ConnectionFailedException *clone () const;
		virtual void rethrow () const;

		QString toString () const;
		QString colorizedString () const;
};

#endif
