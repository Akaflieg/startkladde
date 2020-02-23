/*
 * TODO:
 *   - Handle all relevant errors from
 *     http://dev.mysql.com/doc/refman/5.1/en/error-messages-client.html
 *     http://dev.mysql.com/doc/refman/5.1/en/error-messages-server.html
 *   - Allow retrying (like canceling, kicks the connection, but sets a retry
 *     flag instead of canceled)
 *   - Transactions don't emit an executingQuery signal
 *
 * Improvements:
 *   - On reconnect, don't print "closing" and "connecting" messages (but do
 *     keep printing "Retrying...connecting to ..." on #open
 *   - Colorized transaction output
 *   - Colorized ping output
 *   - Ping could emit a "pinging" signal
 */


/*
 * On synchronization:
 *   - The canceled flag is reset at the beginning of an operation. If the
 *     canceled flag was reset right before the operation, cancelConnection
 *     could be called again before the CanceledException has propagated to the
 *     calling thread, thereby setting canceled again and causing the next
 *     operation to cancel prematurely.
 *     This means that a cancelConnection call *before* the start of the
 *     operation will not cause the operation to cancel. This is consistent
 *     with the fact that the connection (through the proxy) may not have been
 *     established at this point and thus may not be canceled.
 *     This is also plausible as the cancelation by the user is unlikely to
 *     occur before the start of the operation, and even if it is, the user may
 *     cancel again after the start of the operation.
 *     Allowing the operation to be canceled before it has started would
 *     certainly be an improvement, but it is probably quite hard to do without
 *     getting a race condition.
 *     Note that there is still a race condition when calling one method from
 *     another (e. g. open from executeQueryImpl on reconnect) and the cancel
 *     flag is set: the cancel will not be performed because there is no
 *     connection. The use will have to cancel again in this case.
 *
 * On Reconnect:
 *   - Test case: start, connect; low limit (30 Bytes/s) in ThrottleProxy;
 *     disconnect in ThrottleProxy; refresh (connection must be reopened);
 *     cancel (must cancel immediately)
 */
#include "DefaultInterface.h"

#include <iostream>
#include <cassert>

#include <QVariant>
#include <QThread>

// FIXME on windows, can we use mysql/... here? or ... on linux?
#include <errmsg.h>
#include <mysqld_error.h>

#include "src/util/qString.h"
#include "src/db/result/DefaultResult.h"
#include "src/text.h"
#include "src/db/interface/exceptions/QueryFailedException.h"
#include "src/db/interface/exceptions/ConnectionFailedException.h"
#include "src/db/interface/exceptions/PingFailedException.h"
#include "src/db/interface/exceptions/DatabaseDoesNotExistException.h"
#include "src/db/interface/exceptions/AccessDeniedException.h"
#include "src/db/interface/exceptions/TransactionFailedException.h"
#include "src/concurrent/monitor/OperationCanceledException.h"
#include "src/config/Settings.h"
#include "src/concurrent/DefaultQThread.h"
#include "src/i18n/notr.h"

QAtomicInt DefaultInterface::freeNumber=0;

// ******************
// ** Construction **
// ******************

DefaultInterface::DefaultInterface (const DatabaseInfo &dbInfo, int readTimeout):
	Interface (dbInfo),
	displayQueries (Settings::instance ().displayQueries)
{
	proxy=new TcpProxy ();
	proxy->setReadTimeout (readTimeout);

	connect (proxy, SIGNAL (readTimeout ()), this, SIGNAL (readTimeout ()), Qt::DirectConnection);
	connect (proxy, SIGNAL (readResumed ()), this, SIGNAL (readResumed ()), Qt::DirectConnection);

	QString name=qnotr ("startkladde_defaultInterface_%1").arg (getFreeNumber ());

	db=QSqlDatabase::addDatabase (notr ("QMYSQL"), name);
	threadId=QThread::currentThreadId ();
	std::cout << notr ("DefaultInterface created on thread ") << QThread::currentThreadId () << std::endl;
}

DefaultInterface::~DefaultInterface ()
{
	if (db.isOpen ()) db.close ();

	QString name=db.connectionName ();

	// Make sure the QSqlDatabase instance is destroyed before removing it
	db=QSqlDatabase ();
	QSqlDatabase::removeDatabase (name);

	delete proxy;
}


// *******************************
// ** AbstractInterface methods **
// *******************************

/**
 * Opens the connection to the database
 *
 * This is not required as it is done automatically when a query is
 * executed and the connection is not open; however, it can be used to
 * ensure that the database can be reached.
 *
 * @return true on success, false else
 */
bool DefaultInterface::open ()
{
	// Reset the canceled flag
	canceled=0;

	DatabaseInfo info=getInfo ();

	// TODO handle proxy port=0: throw an exception, but we don't have an
	// SqlError, so it's not an SqlException
	quint16 proxyPort=proxy->open (info.server, info.effectivePort ());

	// Note that this may not be "localhost" because the MySQL library uses a
	// unix domain socket and ignores the port in this case.
	db.setHostName     (notr ("127.0.0.1"));
	db.setUserName     (info.username);
	db.setPassword     (info.password);
	db.setPort         (proxyPort);
	db.setDatabaseName (info.database);

	db.setConnectOptions (notr ("CLIENT_COMPRESS"));


	openImpl ();

	return true;
}

void DefaultInterface::openImpl ()
{
	verifyThread ();

	while (true)
	{
		std::cout << qnotr ("%1 connecting to %2 via %3:%4...")
			.arg (db.connectionName ()).arg (getInfo ().toString ()).arg (db.hostName ()).arg (db.port ());
		std::cout.flush ();

		if (db.open ())
		{
			std::cout << notr ("OK") << std::endl;
			return;
		}
		else
		{
			if (canceled)
			{
				// Failed because canceled
				std::cout << notr ("canceled") << std::endl;
				throw OperationCanceledException ();
			}
			else
			{
				// Failed due to error
				QSqlError error=db.lastError ();
				std::cout << error.databaseText () << std::endl;

                int number = extractNativeErrorNumber(error);

                emit databaseError (number, error.databaseText ());

                switch (number)
				{
					case CR_CONN_HOST_ERROR: break; // Retry
					case CR_UNKNOWN_HOST: break; // Retry
					case CR_SERVER_LOST: break; // Retry
					case ER_BAD_DB_ERROR: throw DatabaseDoesNotExistException (error);
					case ER_ACCESS_DENIED_ERROR: throw AccessDeniedException (error);
					case ER_DBACCESS_DENIED_ERROR: throw AccessDeniedException (error);
					case ER_TABLEACCESS_DENIED_ERROR: throw AccessDeniedException (error);
					default: throw ConnectionFailedException (error);
				}

				// Note that if the operation is canceled during this sleep
				// call, it will only end after the sleep has finished.
				// This is acceptable for a delay as short as 1 second. It
				// could be circumvented by using a QWaitCondition.
				DefaultQThread::sleep (1);
				std::cout << notr ("Retrying...");
			}
		}
	}
}

void DefaultInterface::close ()
{
	verifyThread ();

	std::cout << notr ("Closing connection") << std::endl;
	std::cout << notr ("close info ") << getInfo().toString() << std::endl;

	db.close ();
	proxy->close ();
}

QSqlError DefaultInterface::lastError () const
{
	verifyThread ();
	return db.lastError ();
}

/**
 * This method is thread safe.
 */
void DefaultInterface::cancelConnection ()
{
	// Set the flag before calling close, because otherwise, there would be
	// a race condition if the blocking call returns with the canceled flag
	// not yet set.
	canceled=true;
	proxy->close ();
}

void DefaultInterface::transaction ()
{
	transactionStatementImpl (transactionBegin);
}

void DefaultInterface::commit ()
{
	transactionStatementImpl (transactionCommit);
}

void DefaultInterface::rollback ()
{
	transactionStatementImpl (transactionRollback);
}

void DefaultInterface::transactionStatementImpl (AbstractInterface::TransactionStatement statement)
{
	verifyThread ();

	// Reset the canceled flag; make sure this is always done before
	// entering doTransactionStatement.
	canceled=0;

	while (true)
	{
		if (!db.isOpen ()) openImpl ();

		// Keep calling doTransactionStatement until it succeeds. If it failes
		// terminally, it will throw an exception instead of returning false.
		if (doTransactionStatement (statement)) return;

		// The socket may alreday be closed - close the connection so it
		// can be reopened.
		close ();

		if (displayQueries) std::cout << notr ("Retrying...");
	}
}

bool DefaultInterface::doTransactionStatement (TransactionStatement statement)
{
	verifyThread ();

	if (displayQueries) std::cout << transactionStatementString (statement) << notr ("...") << std::flush;

	bool result=false;
	switch (statement)
	{
		case transactionBegin   : result=db.transaction (); break;
		case transactionCommit  : result=db.commit      (); break;
		case transactionRollback: result=db.rollback    (); break;
		// no default
	}

	if (result)
	{
		if (displayQueries) std::cout << notr ("OK") << std::endl;
		return true;
	}
	else if (canceled)
	{
		// Failed because canceled
		if (displayQueries) std::cout << notr ("canceled") << std::endl;
		throw OperationCanceledException ();
	}
	else
	{
		// Failed due to error
		QSqlError error=db.lastError ();
		if (displayQueries) std::cout << error.databaseText () << std::endl;
        int number = extractNativeErrorNumber(error);
        emit databaseError (number, error.databaseText ());

        if (!retryOnQueryError (number))
			throw TransactionFailedException (error, statement);

		return false;
	}
}

/**
 * Executes a query
 *
 * @param query the query to execute
 * @param forwardOnly the forwardOnly flag to set on the query
 * @return a pointer to the result of query; owned by query
 * @throw QueryFailedException if the query fails
 */
void DefaultInterface::executeQuery (const Query &query)
{
	executeQueryImpl (query);
}

/**
 * Executes a query and returns the result
 *
 * @param query the query to execute
 * @param forwardOnly the forwardOnly flag to set on the query
 * @return a QSharedPointer to a DefaultResult encapsulating the QSqlQuery
 * @throw QueryFailedException if the query fails
 */
QSharedPointer<Result> DefaultInterface::executeQueryResult (const Query &query, bool forwardOnly)
{
	QSqlQuery sqlQuery=executeQueryImpl (query, forwardOnly);

	return QSharedPointer<Result> (
		new DefaultResult (sqlQuery));
}

/**
 * Executes a query and returns whether the query had a result (i. e. the
 * result set is not empty)
 *
 * @param query
 * @return
 */
bool DefaultInterface::queryHasResult (const Query &query)
{
	return executeQueryImpl (query, true).size ()>0;
}

bool DefaultInterface::retryOnQueryError (int number)
{
	switch (number)
	{
		case CR_SERVER_GONE_ERROR: return true;
		case CR_SERVER_LOST: return true;
		default: return false;
	}
}

QSqlQuery DefaultInterface::executeQueryImpl (const Query &query, bool forwardOnly)
{
	// Reset the canceled flag; make sure this is always done before
	// entering doExecuteQuery.
	canceled=0;

	while (true)
	{
		try
		{
			if (!db.isOpen ()) openImpl ();

			return doExecuteQuery (query, forwardOnly);
		}
		catch (QueryFailedException &ex)
		{
            int number = extractNativeErrorNumber(ex.error);
            if (!retryOnQueryError (number))
			{
                switch (number)
				{
					case ER_ACCESS_DENIED_ERROR: throw AccessDeniedException (ex.error);
					case ER_DBACCESS_DENIED_ERROR: throw AccessDeniedException (ex.error);
					case ER_TABLEACCESS_DENIED_ERROR: throw AccessDeniedException (ex.error);
					default: throw;
				}
			}

			// The socket may alreday be closed - close the connection so it
			// can be reopened.
			close ();

			if (displayQueries) std::cout << notr ("Retrying...");
		}
	}
}

/**
 * Executes a query and returns the QSqlQuery
 *
 * This method is blocking, but can be canceled by calling cancelConnection
 * from another thread. This method does not use a monitor for a canceled
 * check. This is because
 *   - the canceled check would be the only reason for the monitor (this
 *     method does not report progress)
 *   - passing a monitor here would require passing it through every single
 *     method that somehow ends up calling this, including most of the
 *     methods of Interface (and all implementations). Since the
 *     OperationMonitorInterface would have to be copied by every call,
 *     this is probably faster then with a monitor
 * See also the OperationMonitorInterface class.
 *
 * @throw QueryFailedException if the query fails
 * @throw OperationMonitor::CanceledException if cancelConnection was called
 */
QSqlQuery DefaultInterface::doExecuteQuery (const Query &query, bool forwardOnly)
{
	verifyThread ();

	if (displayQueries)
	{
		std::cout << query.colorizedString () << notr ("...");
		std::cout.flush ();
	}

	QSqlQuery sqlQuery (db);
	sqlQuery.setForwardOnly (forwardOnly);

	emit executingQuery (query);

	if (!query.prepare (sqlQuery))
	{
		if (canceled)
		{
			if (displayQueries)
				std::cout << notr ("canceled") << std::endl;
			throw OperationCanceledException ();
		}
		else
		{
			QSqlError error=sqlQuery.lastError ();
            int number = extractNativeErrorNumber(error);
			if (displayQueries)
				std::cout << error.databaseText () << std::endl;
            emit databaseError (number, error.databaseText ());

			throw QueryFailedException::prepare (error, query);
		}
	}

	query.bindTo (sqlQuery);

	if (displayQueries)
		std::cout << notr ("..."); std::cout.flush ();

	if (!query.exec (sqlQuery))
	{
		if (canceled)
		{
			if (displayQueries)
				std::cout << notr ("canceled") << std::endl;
			throw OperationCanceledException ();
		}
		else
		{
			QSqlError error=sqlQuery.lastError ();
            int number = extractNativeErrorNumber(error);
			if (displayQueries)
				std::cout << error.databaseText () << std::endl;
            emit databaseError (number, error.databaseText ());

			throw QueryFailedException::execute (error, query);
		}
	}
	else
	{
		if (displayQueries)
		{
			int numRows=sqlQuery.size ();

			if (sqlQuery.isSelect ())
				std::cout << countText (numRows,
					notr ("1 row returned"),
					notr ("%1 rows returned")) << std::endl;
			else
				std::cout << countText (numRows,
					notr ("1 row affected"),
					notr ("%1 rows affected")) << std::endl;
		}

		return sqlQuery;
	}
}

void DefaultInterface::ping ()
{
	verifyThread ();

	// Don't flood stdout with ping messages
	bool display=false;
	//bool display=displayQueries;

	// Reset the canceled flag
	canceled=0;

	while (true)
	{
		if (!db.isOpen ()) openImpl ();

		// Keep calling doTransactionStatement until it succeeds. If it failes
		// terminally, it will throw an exception instead of returning false.
		if (display) std::cout << notr ("Ping...") << std::flush;



		QSqlQuery sqlQuery (db);

		if (sqlQuery.prepare (notr ("SELECT 0"))) // Up: 36 bytes, down: 52 bytes
		{
			if (display) std::cout << notr ("OK") << std::endl;
			return;
		}
		else if (canceled)
		{
			// Failed because canceled
			if (display) std::cout << notr ("canceled") << std::endl;
			throw OperationCanceledException ();
		}
		else
		{
			// Failed due to error
			QSqlError error=sqlQuery.lastError ();
            int number = extractNativeErrorNumber(error);
			if (display) std::cout << error.databaseText () << std::endl;
            emit databaseError (number, error.databaseText ());

            if (!retryOnQueryError (number))
				throw PingFailedException (error);
		}

		// The socket may alreday be closed - close the connection so it
		// can be reopened.
		close ();

		if (display) std::cout << notr ("Retrying...");
	}
}

void DefaultInterface::verifyThread () const
{
	// FIXME remove?
	assert (QThread::currentThreadId ()==threadId);

	// The db may only be accessed on the thread where it was created (Qt
	// restriction)
	if (QThread::currentThreadId ()!=threadId)
		std::cout << notr ("FAIL: a method of DefaultInterface was called on the wrong thread!") << std::endl;
}
