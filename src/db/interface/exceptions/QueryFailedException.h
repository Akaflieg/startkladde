/*
 * QueryFailedException.h
 *
 *  Created on: 28.02.2010
 *      Author: Martin Herrmann
 */

#ifndef QUERYFAILEDEXCEPTION_H_
#define QUERYFAILEDEXCEPTION_H_

#include "src/db/Query.h"
#include "src/db/interface/exceptions/SqlException.h"

class QueryFailedException: public SqlException
{
	public:
		enum Phase { phasePrepare, phaseExecute };

		QueryFailedException (const QSqlError &error, const Query &query, Phase phase);
		static QueryFailedException prepare (const QSqlError &error, const Query &query);
		static QueryFailedException execute (const QSqlError &error, const Query &query);

		virtual QueryFailedException *clone () const;
		virtual void rethrow () const;

		QString toString () const;
		QString colorizedString () const;
		static QString phaseString (Phase phase);

		Query query;
		Phase phase;
};

#endif
