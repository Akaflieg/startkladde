/*
 * DefaultResult.h
 *
 *  Created on: 25.02.2010
 *      Author: Martin Herrmann
 */

#ifndef DEFAULTRESULT_H_
#define DEFAULTRESULT_H_

#include <QSqlQuery>

#include "src/db/result/Result.h"
#include "src/i18n/notr.h"

/**
 * A Result implementation that reads the values directly from a
 * QSqlQuery.
 *
 * The QSqlQuery is copied, so the original may be destroyed after
 * creating the Result.
 *
 * Since a QSqlQuery is used, this class may not be accessed in a
 * thread other than the one that created the QSqlQuery. This is a
 * restriction of QtSql.
 */
class DefaultResult: public Result
{
	public:
		// *** Construction
		DefaultResult (QSqlQuery &query);
		virtual ~DefaultResult ();

		// *** Properties
        //QSqlQuery &getQuery ();

		// *** Result methods
		virtual int at () const;
		virtual bool first ();
		virtual bool isNull (int field) const;
		virtual bool last ();
		virtual QVariant lastInsertId () const;
		virtual QString lastQuery () const;
		virtual bool next ();
		virtual int numRowsAffected () const;
		virtual bool previous ();
		virtual QSqlRecord record () const;
		virtual bool seek (int index, bool relative=false);
		virtual int size () const;
		virtual QVariant value (int index) const;

		virtual QString type () const { return notr ("default"); }

	private:
		// Not a reference - make a copy of the query. The query will
		// be destroyed after the query method returns.
        QSqlQuery& query;
};

#endif
