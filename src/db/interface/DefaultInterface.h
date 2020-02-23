/*
 * DefaultInterface.h
 *
 *  Created on: 22.02.2010
 *      Author: Martin Herrmann
 */

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QAtomicInt>

#include "src/db/interface/Interface.h"
#include "src/db/DatabaseInfo.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"
#include "src/net/TcpProxy.h"
#include "src/db/Query.h"

#ifndef DEFAULTINTERFACE_H_
#define DEFAULTINTERFACE_H_

class Query;

/**
 * An Interface implementation directly using a QSqlDatabase
 *
 * As this class uses a QSqlDatabase, not only is it not thread safe,
 * it even may only be used in the thread where it was created. Also,
 * the result returned in the query
 *
 * This is
 * a QtSql restriction, see [1]. For an Interface implementation
 * without this restriction, see ThreadSafeInterface.
 *
 * [1] http://doc.trolltech.com/4.5/threads.html#threads-and-the-sql-module
 */
class DefaultInterface: public QObject, public Interface
{
	Q_OBJECT

	public:
		// *** Construction
		DefaultInterface (const DatabaseInfo &dbInfo, int readTimeout=0);
		DefaultInterface (const DefaultInterface &iface);
		virtual ~DefaultInterface ();

		// *** AbstractInterface methods
		virtual bool open ();
		virtual void close ();
		virtual QSqlError lastError () const;
		virtual void cancelConnection ();
		virtual void transaction ();
		virtual void commit ();
		virtual void rollback ();
		virtual void executeQuery (const Query &query);
		virtual QSharedPointer<Result> executeQueryResult (const Query &query, bool forwardOnly=true);
		virtual bool queryHasResult (const Query &query);
		virtual void ping ();

	signals:
		void executingQuery (Query query);
        void databaseError (int number, QString message);

		void readTimeout ();
		void readResumed ();

	protected:
		void verifyThread () const;

	private:
		// TODO: when accessing db, we want to check the thread. Make a wrapper
		// around db that does this, or better, move db to common base class
		QSqlDatabase db;
		TcpProxy *proxy;
		QAtomicInt canceled; // There is no QAtomicBool. 0=false, others=true
		bool displayQueries;
		Qt::HANDLE threadId; // The thread ID db was created on

		static QAtomicInt freeNumber;
		static int getFreeNumber () { return freeNumber.fetchAndAddOrdered (1); }

		virtual void openImpl ();

		virtual QSqlQuery executeQueryImpl (const Query &query, bool forwardOnly=true);
		virtual QSqlQuery doExecuteQuery (const Query &query, bool forwardOnly=true);

		virtual void transactionStatementImpl (TransactionStatement statement);
		virtual bool doTransactionStatement (TransactionStatement statement);

		virtual bool retryOnQueryError (int number);
};

#endif
