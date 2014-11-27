/*
 * DbManager.h
 *
 *  Created on: 07.03.2010
 *      Author: Martin Herrmann
 */

#ifndef DBMANAGER_H_
#define DBMANAGER_H_

#include <cassert>

#include <QObject>

// TODO many dependencies in header
#include "src/db/DatabaseInfo.h"
#include "src/db/interface/ThreadSafeInterface.h"
#include "src/db/Database.h"
#include "src/db/cache/Cache.h"
#include "src/db/DbWorker.h"
#include "src/db/migration/MigratorWorker.h"
#include "src/db/cache/CacheWorker.h"
#include "src/db/dbId.h"
#include "src/db/interface/InterfaceWorker.h"

class QWidget;

/**
 * Contains the database related objects required in the GUI:
 *   - the Interface
 *   - the Database (ORM)
 *   - the Cache
 * as well as various workers and some functionality related to database
 * management. Specifically, this class contains methods for running
 * asynchronous methods (using a worker class) with a monitor dialog.
 *
 * Advantages of this class:
 *   - worker instances can be reused; e. g. we don't have to create DbWorker
 *     (and the associated thread) each time we want to add or update an
 *     object. Achieving this without a manager would require sharing workers
 *     between several GUI classes, resulting in untidiness
 *   - the setup for running asynchronous methods (which is relatively much
 *     work at the moment) is encapsulated, keeping the GUI classes clean
 *
 * This class is quite heavyweight, so care should be taken in making other
 * classes depend on it. As a rule, non-GUI classes should not use a DbManager,
 * unless there is good reason. GUI classes should access the DbManager by
 * reference.
 *
 * This class is intended to be used from the GUI (in the GUI thread) and as
 * such is not thread safe. Some of the classes contained by the manager are
 * thread safe, though.
 *
 * ATTENTION: all methods which update the database may throw an exception,
 * especially an OperationCanceledException
 */
class DbManager: public QObject
{
	Q_OBJECT

	public:
		enum State { stateDisconnected, stateConnecting, stateConnected };

		class ConnectCanceledException {};

		class ConnectFailedException
		{
			public:
				ConnectFailedException (const QString &message): message (message) {}
				QString message;
		};


		DbManager (const DatabaseInfo &info);
		virtual ~DbManager ();

		virtual ThreadSafeInterface
		                       &getInterface      () { return interface;   }
		virtual Database       &getDb             () { return db;          }
		virtual Cache          &getCache          () { return cache;       }

		virtual DbWorker       &getDbWorker       () { return dbWorker;       }
		virtual MigratorWorker
		                       &getMigratorWorker () { return migratorWorker; }
		virtual CacheWorker    &getCacheWorker    () { return cacheWorker;    }

		virtual State getState () { return state; }


		// *** Schema management
		bool isCurrent (QWidget *parent);
		void ensureCurrent (const QString &message, QWidget *parent);
		bool isEmpty (QWidget *parent);
		void grantPermissions (QWidget *parent);
		void createDatabase (QWidget *parent);
		void createSampleLaunchMethods (QWidget *parent);


		// *** Connection management
		void confirmOrCancel (const QString &title, const QString &question, QWidget *parent);
		void checkVersion (QWidget *parent);
		void openInterface (QWidget *parent);
		void connectImpl (QWidget *parent);

		bool connect (QWidget *parent);
		void disconnect ();

		// *** Data management
		void clearCache ();
		void refreshCache (QWidget *parent);
		void fetchFlights (QDate date, QWidget *parent);
		template<class T> void refreshObjects (QWidget *parent);

		template<class T> bool objectUsed    (dbId id                , QWidget *parent);
		template<class T> void deleteObject  (dbId id                , QWidget *parent);
		template<class T> void deleteObjects (const QList<dbId> &ids , QWidget *parent);
		template<class T> dbId createObject  (      T &object        , QWidget *parent);
		template<class T> void createObjects (      QList<T> &objects, QWidget *parent);
		template<class T> int  updateObject  (const T &object        , QWidget *parent);

		QList<Flight> getFlights (const QDate &first, const QDate &last, QWidget *parent);


		// *** Database updates
		void mergePeople (const Person &correctPerson, const QList<Person> &wrongPeople, QWidget *parent);

	signals:
		void readTimeout ();
		void readResumed ();
		void stateChanged (DbManager::State state);
		void migrationStarted ();
		void migrationEnded ();


	protected:
		void setState (State newState);

		void executeQuery (const Query &query, const QString &statusText, QWidget *parent);
		void transaction (QWidget *parent);
		void commit      (QWidget *parent);
		void rollback    (QWidget *parent);

	protected slots:
		void settingsChanged ();

	private:
		DbManager (const DbManager &other);
		DbManager &operator= (const DbManager &other);

		QString mergeDeleteWarningTitle (int notDeletedCount, int deletedCount);
		QString mergeDeleteWarningText (int notDeletedCount, int deletedCount);

		State state;

		ThreadSafeInterface interface;
		Database db;
		Cache cache;

		InterfaceWorker interfaceWorker;
		DbWorker dbWorker;
		MigratorWorker migratorWorker;
		CacheWorker cacheWorker;

		void doOpenInterface (InterfaceWorker &worker, QWidget *parent);
};

#endif
