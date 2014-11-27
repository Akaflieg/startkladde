/*
 * PingFailedException.h
 *
 *  Created on: 12.03.2010
 *      Author: Martin Herrmann
 */

#ifndef PINGFAILEDEXCEPTION_H_
#define PINGFAILEDEXCEPTION_H_

#include "src/db/interface/exceptions/SqlException.h"

class PingFailedException: public SqlException
{
	public:
		PingFailedException (const QSqlError &error);

		virtual PingFailedException *clone () const;
		virtual void rethrow () const;

		QString toString () const;
		QString colorizedString () const;
};

#endif
