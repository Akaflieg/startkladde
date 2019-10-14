#ifndef SYNC_H
#define SYNC_H

#include <QObject>
#include <QProcess>
#include "src/db/dbId.h"
#include "src/db/Query.h"

class SkProcess;
class SyncDialog;
class DbManager;
class DatabaseInfo;
class Interface;
class Person;
class Flight;
class Plane;
class LaunchMethod;
class QFile;
class QTextStream;
class Query;

class DbSync : public QObject
{
    Q_OBJECT
public:
    enum AsyncWaitingState {
        none,
        waitingForDump,
        waitingForTableSync
    };

    struct Change {
        dbId id;
        QString entity;
        QString command;
        dbId refId;
        QString data;
    };

    class ParsingException: public std::exception
    {
        public:
            ParsingException (QString msg) throw() : message(msg)  { }
            ~ParsingException() throw() { }

            QString message;
    };

    DbSync(DbManager& lclDbManager, SyncDialog* dlg, QObject *parent = 0);
    ~DbSync();

private:
    bool started; //this class can only be used once

    Interface* localDbInterface;
    DatabaseInfo& remoteDbInfo;
    DatabaseInfo& localDbInfo;
    DbManager* localDbManager;
    DbManager* remoteDbManager;
    SkProcess* syncProcess;

    AsyncWaitingState asyncState;
    bool userCancel;
    bool ptConnectFailed;

    //local database dump
    SkProcess* dumpProc;
    QFile* dumpFile;
    QTextStream* dumpOut;
    QString dumpError;

    //logging
    void logMessage(QString msg);
    QStringList syncLog;

    SyncDialog* dialog;
    QList<Change> changes;
    int progress;

    void dumpLocalDb();
    void proceedAfterDump();

    void replaceInsDelInsChainsByUpdate();
    //void removeInsertUpdateDeleteChains();
    template<class T> void applyChanges(Change change);
    template<class T> void constructObjectFromData(T* entity, QString data);

    void checkReferencing(Flight* p);
    void checkReferencing(Plane* p);
    void checkReferencing(Person* p);
    void checkReferencing(LaunchMethod* p);

    static QMap<QString,QString> splitDataString(QString dataString);

signals:
    void completed();
    
public slots:
    void startSynchronisation();

private slots:
    void databaseExecutingQuery(Query query);

    void syncLineReceived(const QString &line);
    void syncErrorLineReceived(const QString &line);
    void syncExited(int exitCode, QProcess::ExitStatus exitStatus);

    void dumpLineReceived(const QString &line);
    void dumpErrorLineReceived(const QString &line);
    void dumpExited(int exitCode, QProcess::ExitStatus exitStatus);

    void dialogClosed();
    void dialogCancelled();
};

#endif // SYNC_H
