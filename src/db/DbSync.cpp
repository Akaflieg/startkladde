#include "DbSync.h"
#include <QFile>
#include <QTextStream>
#include "src/io/SkProcess.h"
#include "src/gui/windows/SyncDialog.h"
#include "src/db/DbManager.h"
#include "src/db/DatabaseInfo.h"
#include "src/config/Settings.h"
#include "src/db/interface/DefaultInterface.h"
#include "src/model/Person.h"
#include "src/model/Flight.h"
#include "src/model/LaunchMethod.h"
#include "src/model/Plane.h"
#include "src/db/cache/Cache.h"
#include "src/db/Query.h"
#include "src/concurrent/monitor/OperationCanceledException.h"

DbSync::DbSync(DbManager& lclDbManager, SyncDialog* dlg, QObject *parent) :
    QObject(parent), remoteDbInfo(Settings::instance().remoteDatabaseInfo), localDbInfo(Settings::instance().databaseInfo)
{
    localDbManager = &lclDbManager;
    remoteDbManager = new DbManager(remoteDbInfo);
    localDbInterface = new DefaultInterface(localDbInfo);
    dialog = dlg;
    progress = 0;
    asyncState = none;
    userCancel = false;
    ptConnectFailed = false;
    started = false;

    connect(dialog, SIGNAL(accepted()), this, SLOT(dialogClosed()));
    connect(dialog, SIGNAL(cancelled()), this, SLOT(dialogCancelled()));
}

DbSync::~DbSync()
{
    delete localDbInterface;
    delete remoteDbManager;
}

void DbSync::dumpLocalDb()
{
    logMessage(notr("Dumping local database..."));
    dialog->setProgress(0, tr("Creating local database backup..."));

    dumpProc = new SkProcess();
    connect(dumpProc, SIGNAL(lineReceived(const QString&)), this, SLOT(dumpLineReceived(const QString)));
    connect(dumpProc, SIGNAL(errorLineReceived(const QString&)), this, SLOT(dumpErrorLineReceived(const QString)));
    connect(dumpProc, SIGNAL(exited(int,QProcess::ExitStatus)), this, SLOT(dumpExited(int,QProcess::ExitStatus)));

    QStringList args;
    args << notr("-u") << localDbInfo.username << qnotr("-p%1").arg(localDbInfo.password) << localDbInfo.database;

    QDir dir (Settings::instance().databaseDumpPath);
    dir.makeAbsolute();

    if (!dir.mkpath(dir.absolutePath()))
    {
        dialog->completed(true, tr("Database dump path could not be created!"));
        logMessage(notr("ABORT. Database dump path could not be created"));
        return;
    }

    dumpFile = new QFile(dir.absoluteFilePath(qnotr("presync_dump_%1.sql").arg(QDateTime::currentDateTime().toString(notr("yyyy-MM-dd-hh-mm-ss")))));
    if (dumpFile->open(QFile::WriteOnly | QFile::Truncate))
    {
        dumpOut = new QTextStream(dumpFile);
    } else {
        dialog->completed(true, tr("Database dump file could not be created!"));
        logMessage(notr("ABORT. Database dump file could not be created"));
        return;
    }

    dumpProc->start(notr("mysqldump"), args);
    asyncState = waitingForDump;
    dialog->setCancelable(true);
}

void DbSync::startSynchronisation()
{
    if (started)
    {
        dialog->completed(true, tr("Internal error!"));
        logMessage(notr("ABORT. Sync class has been used twice"));
        return;
    }

    started = true;
    logMessage(notr("Starting sync..."));

    if (!localDbInfo.different(remoteDbInfo) || localDbInfo.server == remoteDbInfo.server)
    {
        dialog->completed(true, tr("Local and remote database have the same server address!"));
        logMessage(notr("ABORT. Local and remote database have the same server address"));
        return;
    }

    dumpLocalDb();
}

void DbSync::proceedAfterDump()
{
    logMessage(notr("Reading changes from local database..."));
    dialog->setProgress(10, tr("Analyze changes in local database..."));

    //Read changes from database
    QSharedPointer<Result> result;
    try
    {
        localDbInterface->open();
        result = localDbInterface->executeQueryResult(Query(notr("SELECT id, command, entity, ref_id, data FROM changes ORDER BY id ASC;")));
        localDbInterface->close();
    } catch (...)
    {
        dialog->completed(true, tr("Changes could not be retrieved from local database!"));
        logMessage(notr("ABORT. Changes could not be retrieved from local database"));
        return;
    }

    changes.clear();
    while (result->next()) {
        Change ch;
        ch.id = result->value(0).toLongLong();
        ch.command = result->value(1).toString();
        ch.entity = result->value(2).toString();
        ch.refId = result->value(3).toLongLong();
        ch.data = result->value(4).toString();
        changes << ch;
    }

    replaceInsDelInsChainsByUpdate();
    //removeInsertUpdateDeleteChains();

    logMessage(QString(notr("Number of changes: %1")).arg(changes.size()));

    if (changes.size() > 0)
    {
        logMessage(notr("Connecting to remote database..."));
        remoteDbManager->connect(dialog);
        connect (&(remoteDbManager->getInterface()), SIGNAL (executingQuery (Query)), this, SLOT (databaseExecutingQuery (Query)));
        if (remoteDbManager->getState() == DbManager::stateDisconnected)
        {
            dialog->completed(true, tr("Could not connect to remote database!"));
            logMessage(notr("Could not connect to remote database!"));
            return;
        }
        logMessage(notr("Connected to remote database!"));

        for (int i = 0; i < changes.size(); ++i)
        {
            try
            {
                if (changes[i].entity == notr("people")) applyChanges<Person>(changes[i]);
                if (changes[i].entity == notr("flights")) applyChanges<Flight>(changes[i]);
                if (changes[i].entity == notr("launch_methods")) applyChanges<LaunchMethod>(changes[i]);
                if (changes[i].entity == notr("planes")) applyChanges<Plane>(changes[i]);
            }
            catch (OperationCanceledException e)
            {
                dialog->completed(true, tr("Transmission cancelled! Relaunch for complete synchronization!"));
                logMessage(notr("User cancelled transmission of changes to remote database!"));
                remoteDbManager->disconnect();
                return;
            } catch (DbSync::ParsingException e)
            {
                dialog->completed(true, tr("Error during transmission of changes to remote database! Check connection and relaunch!"));
                logMessage(QString(notr("Error during transmission of changes to remote database; ParseException: %1")).arg(e.message));
                remoteDbManager->disconnect();
                return;

            } catch (...)
            {
                dialog->completed(true, tr("Error during transmission of changes to remote database! Check connection and relaunch!"));
                logMessage(notr("Error during transmission of changes to remote database!"));
                remoteDbManager->disconnect();
                return;
            }

            dialog->setProgress(10 + i * (40.0/changes.size()), tr("Transmitting changes to remote database..."));
        }

        remoteDbManager->disconnect();
        logMessage(notr("Disconnected from remote database!"));

        try
        {
            localDbInterface->open();
            localDbInterface->executeQuery(Query(notr("DELETE FROM changes;")));
            localDbInterface->close();
        } catch (...)
        {
            logMessage(notr("WARNING: Change records could not be deleted from local database!"));
        }
    }

    dialog->setProgress(50, tr("Preparing reverse synchronization..."));
    syncProcess = new SkProcess();
    connect(syncProcess, SIGNAL(lineReceived(const QString&)), this, SLOT(syncLineReceived(const QString)));
    connect(syncProcess, SIGNAL(errorLineReceived(const QString&)), this, SLOT(syncErrorLineReceived(const QString)));
    connect(syncProcess, SIGNAL(exited(int,QProcess::ExitStatus)), this, SLOT(syncExited(int,QProcess::ExitStatus)));

    QString processName = notr("pt-table-sync");
    QStringList processArgs;
    processArgs << notr("--execute") << notr("--verbose") << notr("--no-check-triggers");
    processArgs << qnotr("h=%1,u=%2,p=%3,P=3306").arg(remoteDbInfo.server).arg(remoteDbInfo.username).arg(remoteDbInfo.password);
    processArgs << qnotr("--databases=%1").arg(remoteDbInfo.database);
    processArgs << qnotr("h=%1,u=%2,p=%3,P=3306").arg(localDbInfo.server).arg(localDbInfo.username).arg(localDbInfo.password);
    processArgs << qnotr("--databases=%1").arg(localDbInfo.database);
    logMessage(notr("Process pt-table-sync started..."));
    progress = 0;
    syncProcess->start(processName, processArgs);

    asyncState = waitingForTableSync;
    dialog->setCancelable(true);
}

void DbSync::replaceInsDelInsChainsByUpdate()
{
    //Remove chains of the form "update - delete - insert"
    //as are inserted into the changes table by the db trigger
    //and replace them by a simple "update"
    QList<Change> tmp = changes;
    changes.clear();
    for (int i = 0; i < tmp.size(); i++)
    {
        Change ch = tmp[i];

        if (ch.command == "update")
        {
            if (i+2 < tmp.size() &&
                    tmp[i+1].command == "delete" &&
                    tmp[i+2].command == "insert" &&
                    tmp[i+1].refId == tmp[i].refId &&
                    tmp[i+2].refId == tmp[i].refId)
            {
                i += 2;
                ch.data = tmp[i].data;
            }
        }

        changes << ch;
    }
}

/*void DbSync::removeInsertUpdateDeleteChains()
{
    //replace chains like "(insert) - (update) - delete" by
    //a simple "delete" - because that is all that matters
    //TODO not really
    QList<int> refBlackList;

    QList<Change> tmp = changes;
    changes.clear();
    for (int i = tmp.size()-1; i >= 0; i--)
    {
        if (!refBlackList.contains(tmp[i].refId))
            changes.prepend(tmp[i]);
        if (tmp[i].command == "delete")
            refBlackList << tmp[i].refId;
    }
}*/

template<class T> void DbSync::applyChanges(Change change)
{
    if (change.command == "insert") {
        if (change.data.isEmpty()) {
            logMessage(notr("Insert record without data!"));
            return;
        }

        T entity;
        constructObjectFromData<T>(&entity, change.data);

        try {
            checkReferencing(&entity);
        } catch (Cache::NotFoundException e) {
            logMessage(notr("NotFoundException occurred during referencing check on insert"));
        }

        remoteDbManager->updateObject(entity, dialog);
        return;
    }

    if (change.command == "delete") {

        if (!remoteDbManager->objectUsed<T>(change.refId, dialog))
            remoteDbManager->deleteObject<T>(change.refId, dialog);

        return;
    }

    if (change.command == "update") {
        if (change.data.isEmpty()) {
            logMessage(notr("Update record without data!"));
            return;
        }

            T entity;
            constructObjectFromData<T>(&entity, change.data);

            try {
                checkReferencing(&entity);
            } catch (Cache::NotFoundException e) {
                logMessage(notr("NotFoundException occurred during referencing check on update"));
            }

            remoteDbManager->updateObject(entity, dialog);

        return;
    }

    return;
}

void DbSync::checkReferencing(Flight* flight)
{
    Cache& remoteCache = remoteDbManager->getCache();
    Cache& localCache = localDbManager->getCache();

    /*
     * Is there any Person with ID = .. .. ..
     * is there any Plane with ID = .. ..
     * is there any launch_method with ID = ..
     */
    Person pe; Plane pl; LaunchMethod lm;
    if (flight->getPilotId() != 0 && !remoteCache.objectExists(flight->getPilotId(), pe))
    {
        Person p = localCache.getObject<Person>(flight->getPilotId());
        remoteDbManager->updateObject<Person>(p, dialog);
    }
    if (flight->getCopilotId() != 0 && !remoteCache.objectExists(flight->getCopilotId(), pe))
    {
        Person p = localCache.getObject<Person>(flight->getCopilotId());
        remoteDbManager->updateObject<Person>(p, dialog);
    }
    if (flight->getTowpilotId() != 0 && !remoteCache.objectExists(flight->getTowpilotId(), pe))
    {
        Person p = localCache.getObject<Person>(flight->getTowpilotId());
        remoteDbManager->updateObject<Person>(p, dialog);
    }
    if (flight->getPlaneId() != 0 && !remoteCache.objectExists(flight->getPlaneId(), pl))
    {
        Plane p = localCache.getObject<Plane>(flight->getPlaneId());
        remoteDbManager->updateObject<Plane>(p, dialog);
    }
    if (flight->getTowplaneId() != 0 && !remoteCache.objectExists(flight->getTowplaneId(), pl))
    {
        Plane p = localCache.getObject<Plane>(flight->getTowplaneId());
        remoteDbManager->updateObject<Plane>(p, dialog);
    }
    if (flight->getLaunchMethodId() != 0 && !remoteCache.objectExists(flight->getLaunchMethodId(), lm))
    {
        LaunchMethod lm = localCache.getObject<LaunchMethod>(flight->getLaunchMethodId());
        remoteDbManager->updateObject<LaunchMethod>(lm, dialog);
    }
}
void DbSync::checkReferencing(Plane* p) { (void)p; }
void DbSync::checkReferencing(LaunchMethod* lm) { (void)lm; }
void DbSync::checkReferencing(Person *p) { (void)p; }

void DbSync::databaseExecutingQuery (Query query)
{
    logMessage(query.toString());
}

void DbSync::logMessage(QString message)
{
    QString timeString = QTime::currentTime ().toString();
    syncLog << (qnotr ("[%1] %2").arg(timeString).arg(message));
}

void DbSync::syncLineReceived(const QString &line)
{
    logMessage(notr("pt-table-sync: ") + line);
    QStringList tokens = line.split(' ', QString::SkipEmptyParts);
    if (tokens.size() == 10 && tokens[9].contains(notr(".")) && tokens[9] == notr("DATABASE.TABLE"))
    {
        dialog->setProgress(55, tr("Reverse synchronization..."));
    } else if (tokens.size() == 10 && tokens[9].contains(notr(".")) && tokens[9] != notr("DATABASE.TABLE"))
    {
        progress++;
        QString tableName = tokens[9].section('.', -1);
        dialog->setProgress(55 + progress * (45.0/7.0), QString(tr("Table %1 is being updated...")).arg(tableName));
    }
}

void DbSync::syncErrorLineReceived(const QString &line)
{
    logMessage(notr("ERROR in pt-table-sync: ") + line);
    if (line.startsWith(notr("DBI connect")) && line.contains(notr("failed")))
    {
        ptConnectFailed = true;
    }
}

void DbSync::syncExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    (void)exitStatus;

    dialog->setCancelable(false);
    asyncState = none;

    bool error = exitCode == 1 || exitCode == 3 || ptConnectFailed;
    logMessage(notr("Process pt-table-sync finished!"));

    disconnect(syncProcess, SIGNAL(lineReceived(const QString&)), this, SLOT(syncLineReceived(const QString)));
    disconnect(syncProcess, SIGNAL(exited(int,QProcess::ExitStatus)), this, SLOT(syncExited(int,QProcess::ExitStatus)));
    syncProcess->deleteLater();

    try
    {
        logMessage(tr("Remove change records inserted by reverse updates..."));
        localDbInterface->open();
        localDbInterface->executeQuery(Query(notr("DELETE FROM changes;")));
        localDbInterface->close();
    } catch (...)
    {
        logMessage(notr("WARNING: Change records inserted by reverse updates could not be deleted from local database!"));
    }

    if (!error)
    {
        dialog->completed(false, tr("Synchronization successful!"));
        logMessage(notr("Sync finished successful!"));
    } else
    {
        if (userCancel)
        {
            dialog->completed(true, tr("The synchronization has been cancelled by the user!"));
            logMessage(notr("ERROR. Sync finished (pt-table-sync cancelled by the user)"));
        }
        else if (ptConnectFailed)
        {
            dialog->completed(true, tr("The remote database could not be connected!"));
            logMessage(notr("ERROR. Sync finished (pt-table-sync could not connect remote host)"));
        }
        else
        {
            dialog->completed(true, tr("An error occurred during synchronization!"));
            logMessage(notr("ERROR. Sync finished (errors during pt-table-sync)"));
        }
    }

}

void DbSync::dumpLineReceived(const QString& line)
{
    *dumpOut << line << endl;
}

void DbSync::dumpErrorLineReceived(const QString& line)
{
    dumpError.append(line);
    dumpError.append("\n");
}

void DbSync::dumpExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    dialog->setCancelable(false);
    asyncState = none;

    if (userCancel)
        dumpError = tr("Operation cancelled by the user!");

    (void) exitStatus;
    if (exitCode != 0)
    {
        logMessage(notr("ABORT. Database dump failed: ") + dumpError.isEmpty()?"unknown error":dumpError);
        if (!dumpError.isEmpty())
            dialog->completed(true, QString(tr("An error occurred when dumping local database: %1")).arg(dumpError));
        else
            dialog->completed(true, tr("An error occurred when dumping local database!"));
    }

    delete dumpOut;
    dumpProc->deleteLater();
    dumpFile->close();
    dumpFile->deleteLater();

    if (exitCode == 0)
    {
        logMessage(notr("Database dump success!"));
        proceedAfterDump();
    }
}

void DbSync::dialogCancelled()
{
    userCancel = true;

    if (asyncState == waitingForDump)
    {
        dumpProc->stop();
        dumpExited(1, QProcess::CrashExit);
    } else if (asyncState == waitingForTableSync)
    {
        syncProcess->stop();
        syncExited(1, QProcess::CrashExit);
    }

}

void DbSync::dialogClosed()
{
    try
    {
        QDir dir (Settings::instance().databaseDumpPath);
        dir.makeAbsolute();
        QFile logFile (dir.absoluteFilePath(qnotr("sync_log_%1.txt").arg(QDateTime::currentDateTime().toString(notr("yyyy-MM-dd-hh-mm-ss")))));

        if (logFile.open(QFile::WriteOnly | QFile::Truncate))
        {
            QTextStream logFileStream(&logFile);
            for (int i = 0; i < syncLog.size(); i++)
                logFileStream << syncLog[i] << endl;
        }
    } catch (...)
    {
        //Do nothing
    }

    disconnect(dialog, SIGNAL(accepted()), this, SLOT(dialogClosed()));
    disconnect(dialog, SIGNAL(cancelled()), this, SLOT(dialogCancelled()));
    dialog->deleteLater();
    this->deleteLater();
    emit completed();
}

template<class T> void DbSync::constructObjectFromData(T* entity, QString data)
{
    QMap<QString,QString> map = splitDataString(data);
    *entity = T::createFromDataMap(map);
}

QMap<QString, QString> DbSync::splitDataString(QString dataString)
{
    int state = 0;
    /** Finite-State Machine for Parsing
      state 0 = In column identifier, expecting character or =
      state 1 = Expecting [
      state 2 = In length definition, expecting number or ]
      state 3 = In data field, reading n character
      state 5 = expecting character or END.
      **/
    QMap<QString, QString> result;
    QString identBuffer = "";
    QString dataBuffer = "";
    QString lengthBuffer = "";
    int length = 0;
    int dataReadCounter = 0;
    for (int i = 0; i < dataString.length(); i++) {
        QChar c = dataString[i];

        if (state == 0)
        {
            if (c != '=')
                identBuffer.append(c);
            else
                state = 1;
        } else if (state == 1)
        {
            if (c == '[')
                state = 2;
            else
                throw ParsingException(qnotr("Expecting \"[\" but found \"%1\"").arg(c));
        } else if (state == 2)
        {
            if (c != ']')
                lengthBuffer.append(c);
            else {
                if (lengthBuffer == "N")
                {
                    dataBuffer = QString();
                    dataReadCounter = 42;
                    lengthBuffer = "";
                    length = 1;
                    state = 3;
                } else
                {
                    bool success = 0;
                    length = lengthBuffer.toInt(&success);
                    lengthBuffer = "";
                    if (!success) throw ParsingException(notr("Length specifier is not a number"));
                    state = 3;
                }
            }
        } else if (state == 3)
        {
            if (dataReadCounter < length)
            {
                dataBuffer.append(c);
                dataReadCounter++;
            } else {
                if (c != ';')
                    throw ParsingException(notr("Data block is not terminated by a semicolon"));
                dataReadCounter = 0;
                qDebug() << identBuffer << dataBuffer << dataBuffer.isEmpty() << dataBuffer.isNull();
                result.insert(identBuffer, dataBuffer);
                identBuffer = "";
                dataBuffer = "";
                state = 5;
            }
        } else if (state == 5)
        {
            if (c != '=')
            {
                identBuffer.append(c);
                state = 0;
            }
            else
                throw ParsingException(qnotr("Expecting \"=\" but found \"%1\"").arg(c));
        }
    }

    if (state != 5)
        throw ParsingException(notr("Premature end of input string"));

    return result;
}
