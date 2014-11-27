/*
 * CacheWorker.h
 *
 *  Created on: 05.03.2010
 *      Author: Martin Herrmann
 */

#ifndef CACHEWORKER_H_
#define CACHEWORKER_H_

#include <QObject>
#include <QThread>
#include <QDate>

template<typename T> class Returner;
class OperationMonitor;

class Cache;

/**
 * A background worker to perform cache related work in the background
 *
 * All methods return immediately. The result of the operation is
 * returned using a Returner. The operation can be monitored and
 * canceled (if supported by the operation) through an
 * OperationMonitor. returnedValue or wait must be called on the
 * returner after calling the method so exceptions are rethrown.
 *
 * This class is thread safe.
 *
 * See doc/internal/worker.txt
 */
class CacheWorker: public QObject
{
	Q_OBJECT

	public:
		CacheWorker (Cache &cache);
		virtual ~CacheWorker ();

		void refreshAll           (Returner<void> &returner, OperationMonitor &monitor);
		void fetchFlightsOther    (Returner<void> &returner, OperationMonitor &monitor, const QDate &date);
		void refreshPeople        (Returner<void> &returner, OperationMonitor &monitor);
		void refreshPlanes        (Returner<void> &returner, OperationMonitor &monitor);
		void refreshFlights       (Returner<void> &returner, OperationMonitor &monitor);
		void refreshLaunchMethods (Returner<void> &returner, OperationMonitor &monitor);
		void refreshFlarmNetRecords (Returner<void> &returner, OperationMonitor &monitor);

		template<class T> void refreshObjects (Returner<void> &returner, OperationMonitor &monitor);

	signals:
		void sig_refreshAll           (Returner<void> *returner, OperationMonitor *monitor);
		void sig_fetchFlightsOther    (Returner<void> *returner, OperationMonitor *monitor, QDate date);
		void sig_refreshPeople        (Returner<void> *returner, OperationMonitor *monitor);
		void sig_refreshPlanes        (Returner<void> *returner, OperationMonitor *monitor);
		void sig_refreshFlights       (Returner<void> *returner, OperationMonitor *monitor);
		void sig_refreshLaunchMethods (Returner<void> *returner, OperationMonitor *monitor);
		void sig_refreshFlarmNetRecords (Returner<void> *returner, OperationMonitor *monitor);

	protected slots:
		virtual void slot_refreshAll           (Returner<void> *returner, OperationMonitor *monitor);
		virtual void slot_fetchFlightsOther    (Returner<void> *returner, OperationMonitor *monitor, QDate date);
		virtual void slot_refreshPeople        (Returner<void> *returner, OperationMonitor *monitor);
		virtual void slot_refreshPlanes        (Returner<void> *returner, OperationMonitor *monitor);
		virtual void slot_refreshFlights       (Returner<void> *returner, OperationMonitor *monitor);
		virtual void slot_refreshLaunchMethods (Returner<void> *returner, OperationMonitor *monitor);
		virtual void slot_refreshFlarmNetRecords (Returner<void> *returner, OperationMonitor *monitor);

	private:
		QThread thread;
		Cache &cache;
};

#endif
