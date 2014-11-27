/*
 * TransactionFailedException.h
 *
 *  Created on: 28.02.2010
 *      Author: Martin Herrmann
 */

#ifndef TRANSACTIONFAILEDEXCEPTION_H_
#define TRANSACTIONFAILEDEXCEPTION_H_

#include "src/db/interface/AbstractInterface.h" // required for AbstractInterface::TransactionStatement
#include "src/db/interface/exceptions/SqlException.h"

class TransactionFailedException: public SqlException
{
	public:
		TransactionFailedException (const QSqlError &error, AbstractInterface::TransactionStatement statement);

		virtual TransactionFailedException *clone () const;
		virtual void rethrow () const;

		QString toString () const;
		QString colorizedString () const;

		AbstractInterface::TransactionStatement statement;
};

#endif
