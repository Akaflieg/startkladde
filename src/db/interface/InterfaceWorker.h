/*
 * InterfaceWorker.h
 *
 *  Created on: 08.03.2010
 *      Author: Martin Herrmann
 */

#ifndef INTERFACEWORKER_H_
#define INTERFACEWORKER_H_

#include <QObject>
#include <QThread>

#include "src/db/Query.h"

template<typename T> class Returner;
class OperationMonitor;
class ThreadSafeInterface;


class InterfaceWorker: public QObject
{
	Q_OBJECT

	public:
		InterfaceWorker (ThreadSafeInterface &interface);
		virtual ~InterfaceWorker ();

		virtual ThreadSafeInterface &getInterface () { return interface; }

		virtual void open           (Returner<bool> &returner, OperationMonitor &monitor);
		virtual void transaction    (Returner<void> &returner, OperationMonitor &monitor);
		virtual void commit         (Returner<void> &returner, OperationMonitor &monitor);
		virtual void rollback       (Returner<void> &returner, OperationMonitor &monitor);
		virtual void createDatabase (Returner<void> &returner, OperationMonitor &monitor, const QString &name, bool skipIfExists=false);
    	virtual void grantAll       (Returner<void> &returner, OperationMonitor &monitor, const QString &database, const QString &username, const QString &password="");
		virtual void executeQuery   (Returner<void> &returner, OperationMonitor &monitor, const Query &query);

	signals:
		void sig_open           (Returner<bool> *returner, OperationMonitor *monitor);
		void sig_transaction    (Returner<void> *returner, OperationMonitor *monitor);
		void sig_commit         (Returner<void> *returner, OperationMonitor *monitor);
		void sig_rollback       (Returner<void> *returner, OperationMonitor *monitor);
		void sig_createDatabase (Returner<void> *returner, OperationMonitor *monitor, QString name, bool skipIfExists);
    	void sig_grantAll       (Returner<void> *returner, OperationMonitor *monitor, QString database, QString username, QString password);
		void sig_executeQuery   (Returner<void> *returner, OperationMonitor *monitor, Query query);

	protected slots:
		virtual void slot_open           (Returner<bool> *returner, OperationMonitor *monitor);
		virtual void slot_transaction    (Returner<void> *returner, OperationMonitor *monitor);
		virtual void slot_commit         (Returner<void> *returner, OperationMonitor *monitor);
		virtual void slot_rollback       (Returner<void> *returner, OperationMonitor *monitor);
		virtual void slot_createDatabase (Returner<void> *returner, OperationMonitor *monitor, QString name, bool skipIfExists);
    	virtual void slot_grantAll       (Returner<void> *returner, OperationMonitor *monitor, QString database, QString username, QString password);
		virtual void slot_executeQuery   (Returner<void> *returner, OperationMonitor *monitor, Query query);

	private:
		QThread thread;
		ThreadSafeInterface &interface;
};

#endif
