/*
 * CopiedResult.h
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#ifndef COPIEDRESULT_H_
#define COPIEDRESULT_H_

#include <QList>
#include <QVariant>

class QSqlRecord;
class QSqlQuery;

#include "src/db/result/Result.h"
#include "src/i18n/notr.h"

/**
 * A Result implementation that makes a copy of the data.
 *
 * While this class is not thread safe, it may be accessed from a
 * thread other than the one that executed the query.
 */
class CopiedResult: public Result
{
	public:
		CopiedResult (Result &result);
		virtual ~CopiedResult ();

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

		virtual QString type () const { return notr ("copied"); }

	private:
		QList<QSqlRecord> records;
		QString _lastQuery;
		int _numRowsAffected;
		QVariant _lastInsertId;

		int current;
};

#endif
