#ifndef VEREINSFLIEGERSYNCWORKER_H
#define VEREINSFLIEGERSYNCWORKER_H

#include <QObject>
#include <QTreeWidgetItem>
#include "src/db/vfsync/vereinsfliegersync.h"
#include "src/db/DbManager.h"
#include "src/net/Network.h"
#include "src/model/Flight.h"

class VereinsfliegerSyncWorker : public QObject
{
    Q_OBJECT
public:
    explicit VereinsfliegerSyncWorker(DbManager* _dbManager, QString _user, QString _pass, QObject* parent = 0);
    ~VereinsfliegerSyncWorker();

public slots:
    void sync();
    void cancel();
private:
    DbManager* dbManager;
    VereinsfliegerSync* vfsync;
    bool cancelled;
    QString user;
    QString pass;
    QList<QTreeWidgetItem*> errorItems;

    VereinsfliegerFlight convertFlight(Flight& flight);
signals:
    void progress(int progress, QString message);
    void finished(bool errors, QString message, QList<QTreeWidgetItem*> errorItems);
};

#endif // VEREINSFLIEGERSYNCWORKER_H
