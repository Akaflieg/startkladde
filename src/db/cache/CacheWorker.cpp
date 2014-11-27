#include "CacheWorker.h"

#include <iostream>

#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/concurrent/monitor/OperationMonitorInterface.h"
#include "src/db/cache/Cache.h"
#include "src/concurrent/Returner.h"
#include "src/i18n/notr.h"

CacheWorker::CacheWorker (Cache &cache):
	cache (cache)
{
#define CONNECT(definition) connect (this, SIGNAL (sig_ ## definition), this, SLOT (slot_ ## definition))
	CONNECT (refreshAll           (Returner<void>            *, OperationMonitor *));
	CONNECT (fetchFlightsOther    (Returner<void>            *, OperationMonitor *, QDate));
	CONNECT (refreshPeople        (Returner<void>            *, OperationMonitor *));
	CONNECT (refreshPlanes        (Returner<void>            *, OperationMonitor *));
	CONNECT (refreshFlights       (Returner<void>            *, OperationMonitor *));
	CONNECT (refreshLaunchMethods (Returner<void>            *, OperationMonitor *));
	CONNECT (refreshFlarmNetRecords(Returner<void>            *, OperationMonitor *));
#undef CONNECT

	moveToThread (&thread);
	thread.start ();
}

CacheWorker::~CacheWorker ()
{
	thread.quit ();

	std::cout << notr ("Waiting for cache worker thread to terminate...") << std::flush;
	if (thread.wait (1000)) std::cout << notr ("OK")      << std::endl;
	else                    std::cout << notr ("Timeout") << std::endl;
}


// ***********************
// ** Front-end methods **
// ***********************

/**
 * Calls Cache#refreshAll
 */
void CacheWorker::refreshAll (Returner<void> &returner, OperationMonitor &monitor)
{
	emit sig_refreshAll (&returner, &monitor);
}

/**
 * Cache Cache#fetchFlightsOther
 */
void CacheWorker::fetchFlightsOther (Returner<void> &returner, OperationMonitor &monitor, const QDate &date)
{
	emit sig_fetchFlightsOther (&returner, &monitor, date);
}

void CacheWorker::refreshPeople (Returner<void> &returner, OperationMonitor &monitor)
{
	emit sig_refreshPeople (&returner, &monitor);
}

void CacheWorker::refreshPlanes (Returner<void> &returner, OperationMonitor &monitor)
{
	emit sig_refreshPlanes (&returner, &monitor);
}

void CacheWorker::refreshFlights (Returner<void> &returner, OperationMonitor &monitor)
{
	emit sig_refreshFlights (&returner, &monitor);
}

void CacheWorker::refreshFlarmNetRecords (Returner<void> &returner, OperationMonitor &monitor)
{
	emit sig_refreshFlarmNetRecords (&returner, &monitor);
}

void CacheWorker::refreshLaunchMethods (Returner<void> &returner, OperationMonitor &monitor)
{
	emit sig_refreshLaunchMethods (&returner, &monitor);
}


// ********************
// ** Back-end slots **
// ********************

void CacheWorker::slot_refreshAll (Returner<void> *returner, OperationMonitor *monitor)
{
	returnVoidOrException (returner, cache.refreshAll (monitor->interface ()));
}

void CacheWorker::slot_fetchFlightsOther (Returner<void> *returner, OperationMonitor *monitor, QDate date)
{
	returnVoidOrException (returner, cache.fetchFlightsOther (date, monitor->interface ()));
}

void CacheWorker::slot_refreshPeople (Returner<void> *returner, OperationMonitor *monitor)
{
	returnVoidOrException (returner, cache.refreshPeople (monitor->interface ()));
}

void CacheWorker::slot_refreshPlanes (Returner<void> *returner, OperationMonitor *monitor)
{
	returnVoidOrException (returner, cache.refreshPlanes (monitor->interface ()));
}

void CacheWorker::slot_refreshFlights (Returner<void> *returner, OperationMonitor *monitor)
{
	returnVoidOrException (returner, cache.refreshFlights (monitor->interface ()));
}

void CacheWorker::slot_refreshFlarmNetRecords (Returner<void> *returner, OperationMonitor *monitor)
{
	returnVoidOrException (returner, cache.refreshFlarmNetRecords (monitor->interface ()));
}

void CacheWorker::slot_refreshLaunchMethods (Returner<void> *returner, OperationMonitor *monitor)
{
	returnVoidOrException (returner, cache.refreshLaunchMethods (monitor->interface ()));
}


// *****************************
// ** Template specialization **
// *****************************

// This is a hack to avoid having to add tasks (like in DbWorker) here.

template<> void CacheWorker::refreshObjects<Person> (Returner<void> &returner, OperationMonitor &monitor)
{
	refreshPeople (returner, monitor);
}

template<> void CacheWorker::refreshObjects<Plane> (Returner<void> &returner, OperationMonitor &monitor)
{
	refreshPlanes (returner, monitor);
}

template<> void CacheWorker::refreshObjects<Flight> (Returner<void> &returner, OperationMonitor &monitor)
{
	refreshFlights (returner, monitor);
}

template<> void CacheWorker::refreshObjects<FlarmNetRecord> (Returner<void> &returner, OperationMonitor &monitor)
{
	refreshFlarmNetRecords (returner, monitor);
}

template<> void CacheWorker::refreshObjects<LaunchMethod> (Returner<void> &returner, OperationMonitor &monitor)
{
	refreshLaunchMethods (returner, monitor);
}
