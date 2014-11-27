/*
 * AccessDeniedException.h
 *
 *  Created on: 02.03.2010
 *      Author: Martin Herrmann
 */

#ifndef ACCESSDENIEDEXCEPTION_H_
#define ACCESSDENIEDEXCEPTION_H_

#include "src/db/interface/exceptions/ConnectionFailedException.h"

class AccessDeniedException: public ConnectionFailedException
{
	public:
		AccessDeniedException (const QSqlError &error);

		virtual AccessDeniedException *clone () const;
		virtual void rethrow () const;
};

#endif
