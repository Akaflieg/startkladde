/*
 * DbWorker.h
 *
 *  Created on: 05.03.2010
 *      Author: Martin Herrmann
 */

#ifndef DBWORKER_H_
#define DBWORKER_H_

#include <QObject>
#include <QThread>
#include <QList>

#include "src/db/dbId.h"
#include "src/db/Query.h"
#include "src/concurrent/Returner.h"
#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"


class Database;

/**
 * This class is thread safe.
 *
 * All methods taking a reference to an object may access the object
 * asynchronously. The object must not be destroyed until the task has ended.
 *
 * See doc/internal/worker.txt
 */
class DbWorker: QObject
{
	Q_OBJECT

	public:
		/** Implementation detail, please disregard */
		class Task
		{
			public:
				virtual ~Task () {}
				virtual void run (Database &db, OperationMonitor *monitor)=0;
		};

		DbWorker (Database &db);
		virtual ~DbWorker ();

		template<class T> void getObjects    (Returner<QList<T> > &returner, OperationMonitor &monitor, const Query &condition);
		template<class T> void createObject  (Returner<dbId     > &returner, OperationMonitor &monitor, T &object);
		template<class T> void createObjects (Returner<void     > &returner, OperationMonitor &monitor, QList<T> &objects);
		template<class T> void deleteObject  (Returner<bool     > &returner, OperationMonitor &monitor, dbId id);
		template<class T> void deleteObjects (Returner<int      > &returner, OperationMonitor &monitor, const QList<dbId> &ids);
		template<class T> void updateObject  (Returner<bool     > &returner, OperationMonitor &monitor, const T &object);
		template<class T> void objectUsed    (Returner<bool     > &returner, OperationMonitor &monitor, dbId id);

	protected:
		virtual void executeAndDeleteTask (OperationMonitor *monitor, Task *task);

	signals:
		void sig_executeAndDeleteTask (OperationMonitor *monitor, Task *task);

	protected slots:
		virtual void slot_executeAndDeleteTask (OperationMonitor *monitor, Task *task);


	private:
		QThread thread;
		Database &db;
};

#endif
