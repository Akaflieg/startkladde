/*
 * AbstractInterface.h
 *
 *  Created on: 25.02.2010
 *      Author: Martin Herrmann
 */

#ifndef ABSTRACTINTERFACE_H_
#define ABSTRACTINTERFACE_H_

#include <QSharedPointer>
#include <QSqlError>

#include "src/db/DatabaseInfo.h"
#include "src/db/result/Result.h" // Required for deletion of QSharedPointer<Result>

class Query;

/**
 * A low level interface to the database, capable of managing a
 * connection and executing queries.
 *
 * It also provides abstractions for different database backends (even
 * though QSqlDatabase is used, there are still some differences
 * between backends, for example the connection parameters and the data
 * types).
 */
class AbstractInterface
{
	public:
		enum TransactionStatement { transactionBegin, transactionCommit, transactionRollback };

		AbstractInterface (const DatabaseInfo &info);
		virtual ~AbstractInterface ();

		// *** Connection management
		virtual bool open ()=0;
		virtual void close ()=0;
		virtual QSqlError lastError () const=0;

		virtual const DatabaseInfo &getInfo () const;
		virtual void setInfo (const DatabaseInfo &info);

		/**
		 * Makes the current database call terminate immediately, if
		 * possible
		 *
		 * This is intended as a way to cancel operations over a slow
		 * or broken network connection. It may not have any effect
		 * with some Interface implementations, for example with an
		 * on-disk database.
		 *
		 * It is up to the method that called the Interface method to
		 * cancel the operation via a monitor, and the Interface method
		 * to be cancelable.
		 *
		 * This method will generally not be called by the Interface's
		 * thread's event loop as it is intended to be used when the
		 * thread is blocking in a network call.
		 *
		 * Implementations of this method must be thread safe.
		 */
		virtual void cancelConnection ()=0;

		// *** Transactions
		virtual void transaction ()=0;
		virtual void commit ()=0;
		virtual void rollback ()=0;
		static QString transactionStatementString (TransactionStatement statement);

		// *** Queries
		virtual void executeQuery (const Query &query)=0;
		virtual QSharedPointer<Result> executeQueryResult (const Query &query, bool forwardOnly=true)=0;
		/** Not implemented by means of executeQueryResult for efficiency reasons */
		virtual bool queryHasResult (const Query &query)=0;
		virtual void ping ()=0;



	private:
		DatabaseInfo info;
};

#endif
