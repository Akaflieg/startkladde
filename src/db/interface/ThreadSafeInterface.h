/*
 * ThreadSafeInterface.h
 *
 *  Created on: 26.02.2010
 *      Author: Martin Herrmann
 */

#ifndef THREADSAFEINTERFACE_H_
#define THREADSAFEINTERFACE_H_

#include <QObject>
#include <QThread>
#include <QTimer>

#include "src/db/interface/Interface.h"
#include "src/db/Query.h" // required for passing a query by copy in a signal

class QSqlError;

template<typename T> class Returner;
class OperationMonitor;

/**
 * The implementation of this class is similar to the one described in
 * doc/internal/worker.txt
 */
class ThreadSafeInterface: public QObject, public Interface
{
	Q_OBJECT

	public:
		// *** Construction
		ThreadSafeInterface (const DatabaseInfo &info, int readTimeout=0, int keepaliveInterval=0);
		virtual ~ThreadSafeInterface ();


		// *** Monitoring
		virtual bool event (QEvent *e);


	public:
		// *** Frontend methods
		virtual void setInfo (const DatabaseInfo &info);
		virtual bool open ();
		virtual void close ();
		virtual QSqlError lastError () const;
		virtual void transaction ();
		virtual void commit ();
		virtual void rollback ();
		virtual void executeQuery (const Query &query);
		virtual QSharedPointer<Result> executeQueryResult (const Query &query, bool forwardOnly=true);
		virtual bool queryHasResult (const Query &query);
		virtual void ping ();
		virtual void setKeepaliveEnabled (bool enabled);

	public slots:
		virtual void cancelConnection ();

	signals:
		// *** Worker signals
		void sig_setInfo   (Returner<void>      *returner, DatabaseInfo info);
		void sig_open      (Returner<bool>      *returner);
		void sig_close     (Returner<void>      *returner);
		void sig_lastError (Returner<QSqlError> *returner) const;
		void sig_transaction (Returner<void> *returner);
		void sig_commit      (Returner<void> *returner);
		void sig_rollback    (Returner<void> *returner);
		void sig_executeQuery       (Returner<void>                    *returner, Query query);
		void sig_executeQueryResult (Returner<QSharedPointer<Result> > *returner, Query query, bool forwardOnly=true);
		void sig_queryHasResult     (Returner<bool>                    *returner, Query query);
		void sig_ping               (Returner<void>                    *returner);


		void executingQuery (Query query);
		void databaseError (int number, QString message);

		void readTimeout ();
		void readResumed ();

	protected:
		void keepalive ();

	protected slots:
		// *** Backend slots
		virtual void slot_createInterface ();
		virtual void slot_setInfo   (Returner<void>      *returner, DatabaseInfo info);
		virtual void slot_open      (Returner<bool>      *returner);
		virtual void slot_close     (Returner<void>      *returner);
		virtual void slot_lastError (Returner<QSqlError> *returner) const;
		virtual void slot_transaction (Returner<void> *returner);
		virtual void slot_commit      (Returner<void> *returner);
		virtual void slot_rollback    (Returner<void> *returner);
		virtual void slot_executeQuery       (Returner<void>                    *returner, Query query);
		virtual void slot_executeQueryResult (Returner<QSharedPointer<Result> > *returner, Query query, bool forwardOnly=true);
		virtual void slot_queryHasResult     (Returner<bool>                    *returner, Query query);
		virtual void slot_ping               (Returner<void>                    *returner);

		void startKeepaliveTimer ();
		void stopKeepaliveTimer ();
		virtual void keepaliveTimer_timeout ();

	private:
		int readTimeoutSeconds;
		bool keepaliveEnabled;
		int keepaliveInterval; // milliseconds
		QTimer keepaliveTimer;
		QThread thread;
		AbstractInterface *interface;
		bool isOpen;
};

#endif
