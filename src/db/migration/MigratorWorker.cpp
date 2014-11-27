#include "MigratorWorker.h"

#include <iostream>

#include "src/db/migration/Migrator.h"
#include "src/concurrent/monitor/OperationMonitor.h"
#include "src/db/interface/ThreadSafeInterface.h" // required for Interface inheritance
#include "src/i18n/notr.h"

MigratorWorker::MigratorWorker (ThreadSafeInterface &interface):
	migrator (interface)
{
#define CONNECT(definition) connect (this, SIGNAL (sig_ ## definition), this, SLOT (slot_ ## definition))
	CONNECT (migrate           (Returner<void>             *, OperationMonitor *));
	CONNECT (loadSchema        (Returner<void>             *, OperationMonitor *));
	CONNECT (pendingMigrations (Returner<QList<quint64> >  *, OperationMonitor *));
	CONNECT (isCurrent         (Returner<bool>             *, OperationMonitor *));
	CONNECT (isEmpty           (Returner<bool>             *, OperationMonitor *));
	CONNECT (currentVersion    (Returner<quint64>          *, OperationMonitor *));
	CONNECT (getRequiredAction (Returner<Migrator::Action> *, OperationMonitor *, quint64 *, int *));
#undef CONNECT

	moveToThread (&thread);
	thread.start ();
}

MigratorWorker::~MigratorWorker ()
{
	thread.quit ();

	std::cout << notr ("Waiting for migrator worker thread to terminate...") << std::flush;
	if (thread.wait (1000)) std::cout << notr ("OK")      << std::endl;
	else                    std::cout << notr ("Timeout") << std::endl;
}

// ***********************
// ** Front-end methods **
// ***********************

void MigratorWorker::migrate (Returner<void> &returner, OperationMonitor &monitor)
{
	emit sig_migrate (&returner, &monitor);
}

void MigratorWorker::loadSchema (Returner<void> &returner, OperationMonitor &monitor)
{
	emit sig_loadSchema (&returner, &monitor);
}

void MigratorWorker::pendingMigrations (Returner<QList<quint64> > &returner, OperationMonitor &monitor)
{
	emit sig_pendingMigrations (&returner, &monitor);
}

void MigratorWorker::isCurrent (Returner<bool> &returner, OperationMonitor &monitor)
{
	emit sig_isCurrent (&returner, &monitor);
}

void MigratorWorker::isEmpty (Returner<bool> &returner, OperationMonitor &monitor)
{
	emit sig_isEmpty (&returner, &monitor);
}

void MigratorWorker::currentVersion (Returner<quint64> &returner, OperationMonitor &monitor)
{
	emit sig_currentVersion (&returner, &monitor);
}

void MigratorWorker::getRequiredAction (Returner<Migrator::Action> &returner, OperationMonitor &monitor, quint64 *currentVersion, int *numPendingMigrations)
{
	emit sig_getRequiredAction (&returner, &monitor, currentVersion, numPendingMigrations);
}


// ********************
// ** Back-end slots **
// ********************

void MigratorWorker::slot_migrate (Returner<void> *returner, OperationMonitor *monitor)
{
	emit migrationStarted ();
	dontReturnVoidOrException (returner, migrator.migrate (monitor->interface ()));
	emit migrationEnded ();
}

void MigratorWorker::slot_loadSchema (Returner<void> *returner, OperationMonitor *monitor)
{
	returnVoidOrException (returner, migrator.loadSchema (monitor->interface ()));
}

void MigratorWorker::slot_pendingMigrations (Returner<QList<quint64> > *returner, OperationMonitor *monitor)
{
	OperationMonitorInterface interface=monitor->interface ();
	returnOrException (returner, migrator.pendingMigrations ());
}

void MigratorWorker::slot_isCurrent (Returner<bool> *returner, OperationMonitor *monitor)
{
	returnOrException (returner, migrator.isCurrent (monitor->interface ()));
}

void MigratorWorker::slot_isEmpty (Returner<bool> *returner, OperationMonitor *monitor)
{
	returnOrException (returner, migrator.isEmpty (monitor->interface ()));
}

void MigratorWorker::slot_currentVersion (Returner<quint64> *returner, OperationMonitor *monitor)
{
	OperationMonitorInterface interface=monitor->interface ();
	returnOrException (returner, migrator.currentVersion ());
}

void MigratorWorker::slot_getRequiredAction (Returner<Migrator::Action> *returner, OperationMonitor *monitor, quint64 *currentVersion, int *numPendingMigrations)
{
	returnOrException (returner, migrator.getRequiredAction (currentVersion, numPendingMigrations, monitor->interface ()));
}
