#include "DbWorker.h"

#include <iostream>

#include "src/db/Database.h"

#include "src/model/Person.h"
#include "src/model/LaunchMethod.h"
#include "src/model/Plane.h"
#include "src/model/Flight.h"
#include "src/flarm/flarmNet/FlarmNetRecord.h" 
#include "src/i18n/notr.h"

/*
 * Now, the implementation of this here worker class is a bit more complicated
 * than that of other worker classes, due to the fact that we need template
 * methods (e. g. addObject), but cannot have template signals/slots. Thus, we
 * have to resort to functors (class Task).
 */

// ***********
// ** Tasks **
// ***********

template<class T> class GetObjectsTask: public DbWorker::Task
{
	public:
		GetObjectsTask (Returner<QList<T> > *returner, const Query &condition):
			returner (returner), condition (condition)
		{
		}

		virtual ~GetObjectsTask () {}

		Returner<QList <T> > *returner;
		// FIXME all of the tasks are handed to the worker thread, so they
		// should not store any references if possible. Where not possible,
		// document that it must not be called with a reference to a temporary
		// (i. e. the result of a function called in the argument to this
		// method).
		const Query &condition;
//		Query condition;

		virtual void run (Database &db, OperationMonitor *monitor)
		{
			OperationMonitorInterface interface=monitor->interface ();
			returnOrException (returner, db.getObjects<T> (condition));
		}
};

template<class T> class CreateObjectTask: public DbWorker::Task
{
	public:
		CreateObjectTask (Returner<dbId> *returner, T &object):
			returner (returner), object (object)
		{
		}

		virtual ~CreateObjectTask () {}

		Returner<dbId> *returner;
		T &object;

		virtual void run (Database &db, OperationMonitor *monitor)
		{
			OperationMonitorInterface interface=monitor->interface ();
			returnOrException (returner, db.createObject (object));
		}
};

template<class T> class CreateObjectsTask: public DbWorker::Task
{
	public:
		CreateObjectsTask (Returner<void> *returner, QList<T> &objects):
			returner (returner), objects (objects)
		{
		}

		virtual ~CreateObjectsTask () {}

		Returner<void> *returner;
		QList<T> &objects;

		virtual void run (Database &db, OperationMonitor *monitor)
		{
			returnVoidOrException (returner, db.createObjects (objects, monitor->interface ()));
		}
};

template<class T> class DeleteObjectTask: public DbWorker::Task
{
	public:
		DeleteObjectTask (Returner<bool> *returner, dbId id):
			returner (returner), id (id)
		{
		}

		virtual ~DeleteObjectTask () {}

		Returner<bool> *returner;
		dbId id;

		virtual void run (Database &db, OperationMonitor *monitor)
		{
			OperationMonitorInterface interface=monitor->interface ();
			returnOrException (returner, db.deleteObject<T> (id));
		}
};

template<class T> class DeleteObjectsTask: public DbWorker::Task
{
	public:
		DeleteObjectsTask (Returner<int> *returner, const QList<dbId> &ids):
			returner (returner), ids (ids)
		{
		}

		virtual ~DeleteObjectsTask () {}

		Returner<int> *returner;
		const QList<dbId> &ids;

		virtual void run (Database &db, OperationMonitor *monitor)
		{
			OperationMonitorInterface interface=monitor->interface ();
			returnOrException (returner, db.deleteObjects<T> (ids));
		}
};

template<class T> class UpdateObjectTask: public DbWorker::Task
{
	public:
		UpdateObjectTask (Returner<bool> *returner, const T &object):
			returner (returner), object (object)
		{
		}

		virtual ~UpdateObjectTask () {}

		Returner<bool> *returner;
		const T &object;

		virtual void run (Database &db, OperationMonitor *monitor)
		{
			OperationMonitorInterface interface=monitor->interface ();
			returnOrException (returner, db.updateObject (object));
		}
};

template<class T> class ObjectUsedTask: public DbWorker::Task
{
	public:
		ObjectUsedTask (Returner<bool> *returner, dbId id):
			returner (returner), id (id)
		{
		}

		virtual ~ObjectUsedTask () {}

		Returner<bool> *returner;
		const dbId id;

		virtual void run (Database &db, OperationMonitor *monitor)
		{
			OperationMonitorInterface interface=monitor->interface ();
			returnOrException (returner, db.objectUsed<T> (id));
		}
};



// ******************
// ** Construction **
// ******************

DbWorker::DbWorker (Database &db):
	db (db)
{
#define CONNECT(definition) connect (this, SIGNAL (sig_ ## definition), this, SLOT (slot_ ## definition))
	CONNECT (executeAndDeleteTask (OperationMonitor *, Task *));
#undef CONNECT

	moveToThread (&thread);
	thread.start ();
}

DbWorker::~DbWorker ()
{
	thread.quit ();

	std::cout << notr ("Waiting for ORM worker thread to terminate...") << std::flush;
	if (thread.wait (1000)) std::cout << notr ("OK")      << std::endl;
	else                    std::cout << notr ("Timeout") << std::endl;
}

// ***********************
// ** Front-end methods **
// ***********************

template<class T> void DbWorker::getObjects (Returner<QList<T> > &returner, OperationMonitor &monitor, const Query &condition)
{
	executeAndDeleteTask (&monitor, new GetObjectsTask<T> (&returner, condition));
}

template<class T> void DbWorker::createObject (Returner<dbId> &returner, OperationMonitor &monitor, T &object)
{
	executeAndDeleteTask (&monitor, new CreateObjectTask<T> (&returner, object));
}

template<class T> void DbWorker::createObjects (Returner<void> &returner, OperationMonitor &monitor, QList<T> &objects)
{
	executeAndDeleteTask (&monitor, new CreateObjectsTask<T> (&returner, objects));
}

template<class T> void DbWorker::deleteObject (Returner<bool> &returner, OperationMonitor &monitor, dbId id)
{
	executeAndDeleteTask (&monitor, new DeleteObjectTask<T> (&returner, id));
}

template<class T> void DbWorker::deleteObjects (Returner<int> &returner, OperationMonitor &monitor, const QList<dbId> &ids)
{
	executeAndDeleteTask (&monitor, new DeleteObjectsTask<T> (&returner, ids));
}

template<class T> void DbWorker::updateObject (Returner<bool> &returner, OperationMonitor &monitor, const T &object)
{
	executeAndDeleteTask (&monitor, new UpdateObjectTask<T> (&returner, object));
}

template<class T> void DbWorker::objectUsed (Returner<bool> &returner, OperationMonitor &monitor, dbId id)
{
	executeAndDeleteTask (&monitor, new ObjectUsedTask<T> (&returner, id));
}

void DbWorker::executeAndDeleteTask (OperationMonitor *monitor, DbWorker::Task *task)
{
	emit sig_executeAndDeleteTask (monitor, task);
}


// ********************
// ** Back-end slots **
// ********************

void DbWorker::slot_executeAndDeleteTask (OperationMonitor *monitor, DbWorker::Task *task)
{
	task->run (db, monitor);
	delete task;
}


#define INSTANTIATE_TEMPLATES(T) \
	template class CreateObjectTask<T>; \
	template void DbWorker::getObjects    <T> (Returner<QList <T> > &returner, OperationMonitor &monitor, const Query &condition); \
	template void DbWorker::createObject  <T> (Returner<dbId>       &returner, OperationMonitor &monitor, T &object); \
	template void DbWorker::createObjects <T> (Returner<void>       &returner, OperationMonitor &monitor, QList<T> &object); \
	template void DbWorker::deleteObject  <T> (Returner<bool>       &returner, OperationMonitor &monitor, dbId id); \
	template void DbWorker::deleteObjects <T> (Returner<int >       &returner, OperationMonitor &monitor, const QList<dbId> &ids); \
	template void DbWorker::updateObject  <T> (Returner<bool>       &returner, OperationMonitor &monitor, const T &object); \
	template void DbWorker::objectUsed    <T> (Returner<bool>       &returner, OperationMonitor &monitor, dbId id); \
	// Empty line

INSTANTIATE_TEMPLATES (Person      )
INSTANTIATE_TEMPLATES (Plane       )
INSTANTIATE_TEMPLATES (Flight      )
INSTANTIATE_TEMPLATES (LaunchMethod)
INSTANTIATE_TEMPLATES (FlarmNetRecord)

#undef INSTANTIATE_TEMPLATES
