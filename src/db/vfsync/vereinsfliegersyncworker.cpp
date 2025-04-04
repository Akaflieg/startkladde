#include "vereinsfliegersyncworker.h"
#include "src/net/Network.h"
#include "src/config/Settings.h"

VereinsfliegerSyncWorker::VereinsfliegerSyncWorker(DbManager* _dbManager, QString _user, QString _pass, QObject* parent) : QObject(parent)
{
    dbManager = _dbManager;
    user = _user;
    pass = _pass;
    vfsync = nullptr;
    cancelled = false;
}

VereinsfliegerSyncWorker::~VereinsfliegerSyncWorker()
{
    if (vfsync != nullptr) {
        delete vfsync;
    }
}

void VereinsfliegerSyncWorker::cancel()
{
    if (vfsync != nullptr) { // just safety
        vfsync->cancel();
    }
    cancelled = true;
}

void VereinsfliegerSyncWorker::sync()
{
    errorItems.clear();

    Settings& settings = Settings::instance();
    if (!settings.vfUploadEnabled) {
        emit finished(true, tr("Vereinsflieger upload is not enabled."), errorItems);
        return;
    }

    VfCredentials creds;
    creds.user = user;
    creds.pass = pass;
    QList<QString> cidStrList = settings.vfClubId.split(",", Qt::SkipEmptyParts);

    foreach (QString cidStr, cidStrList) {
        bool ok = false;
        int cid = cidStr.toInt(&ok);
        if (ok) {
            creds.cidList.append(cid);
        } else {
            emit finished(true, tr("Invalid club id specified in the settings."), errorItems);
        }
    }

    if (settings.vfApiKey.isEmpty()) {
        emit finished(true, tr("No Vereinsflieger API key specified in the settings."), errorItems);
        return;
    } else {
        creds.appkey = settings.vfApiKey;
    }

    if (cancelled) {
        emit finished(true, tr("Operation cancelled by user."), errorItems);
        return;
    }

    if (vfsync != nullptr) {
        delete vfsync;
    }
    vfsync = new VereinsfliegerSync(Network::getNetworkAccessManager(), creds, this);

    if (vfsync->retrieveAccesstoken() != 0) {
        emit finished(true, tr("Connection to Vereinsflieger could not be initiated."), errorItems);
        return;
    }

    if (cancelled) {
        emit finished(true, tr("Operation cancelled by user."), errorItems);
        return;
    }

    emit progress(0, tr("Signing in..."));
    bool signedIn = vfsync->signin();

    if (!signedIn) {
        emit finished(true, tr("Failed to sign in on Vereinsflieger."), errorItems);
        return;
    }

    if (cancelled) {
        emit finished(true, tr("Operation cancelled by user."), errorItems);
        return;
    }

    emit progress(0, tr("Fetching flights..."));
    QList<Flight> flights = dbManager->getDb().getFlightsUnimportedVF();
    QMap<dbId, Flight> flightMap;
    QList<VereinsfliegerFlight> vfFlights;

    foreach(Flight flight, flights) {
        VereinsfliegerFlight vfFlight (flight, dbManager);
        vfFlights.append(vfFlight);
        flightMap.insert(flight.getId(), flight);
    }

    QString proxyScriptCmd = settings.vfProxyScript;
    if (!proxyScriptCmd.isNull() && !proxyScriptCmd.trimmed().isEmpty()) {

        QStringList tokens = proxyScriptCmd.split(" ", Qt::SkipEmptyParts);

        if (!tokens.isEmpty()) {

            emit progress(0, tr("Calling proxy script..."));

            QJsonArray jsonArray;
            foreach(VereinsfliegerFlight f, vfFlights) {
                QJsonObject flightJson;
                f.writeJson(flightJson);
                jsonArray.append(flightJson);
            }

            QProcess process;
            process.setProgram(tokens.at(0));
            process.setArguments(tokens.mid(1));
            process.start();
            process.write(QJsonDocument(jsonArray).toJson());
            process.closeWriteChannel();
            process.waitForFinished();
            QByteArray response = process.readAllStandardOutput();

            if (process.state() == QProcess::Running || process.state() == QProcess::Starting) {
                emit finished(true, tr("Proxy script is still running, does it maybe contain an endless loop?"), errorItems);
                return;
            }

            if (process.error() == QProcess::FailedToStart) {
                emit finished(true, tr("Proxy script could not be executed."), errorItems);
                return;
            }

            if (process.exitStatus() != QProcess::NormalExit) {
                emit finished(true, tr("Proxy script crashed."), errorItems);
                return;
            }

            if (process.exitCode() != 0) {
                emit finished(true, tr("Proxy script terminated with an erroneous exit code (different from 0)."), errorItems);
                return;
            }

            QJsonDocument doc = QJsonDocument::fromJson(response);

            vfFlights.clear();
            for (int i = 0; i < doc.array().size(); i++) {
                QJsonValue v = doc.array().at(i);
                VereinsfliegerFlight f;
                f.readJson(v.toObject());
                vfFlights.append(f);
            }
        }
    }

    int counter = 0;
    int successCounter = 0;
    foreach(VereinsfliegerFlight vfFlight, vfFlights) {
        counter++;
        Flight dbFlight = flightMap.value(vfFlight.id);

        if (dbFlight.getVfId () == 0) {
            emit progress(((double) counter / (double) flights.size())*1000, QString(tr("Uploading flight %1 of %2...")).arg(counter).arg(flights.size()));
        } else {
            emit progress(((double) counter / (double) flights.size())*1000, QString(tr("Updating flight %1 of %2...")).arg(counter).arg(flights.size()));
        }

        try {

            if (dbFlight.getVfId() == 0) {
                vfsync->addflight(vfFlight);
            } else {
                vfsync->editflight(vfFlight);
            }

            dbFlight.setVfId(vfFlight.vfid);
            dbFlight.setUploaded (true);

            dbManager->getDb().updateObject<Flight>(dbFlight);

            successCounter++;
        } catch (VfSyncException ex) {
            QTreeWidgetItem* item = new QTreeWidgetItem();
            item->setText(0, vfFlight.pilotname);
            item->setText(1, vfFlight.attendantname);
            item->setText(2, vfFlight.departuretime.toString("dd.MM.yyyy"));
            item->setText(3, vfFlight.departuretime.toString("HH:mm"));
            item->setText(4, vfFlight.arrivaltime.toString("HH:mm"));
            item->setText(5, ex.replyString);

            /*QJsonDocument doc = QJsonDocument::fromJson(ex.replyString.toUtf8());

            if (ex.cancelled) {
                item->setText(5, tr("Connection cancelled!"));
            } else {
                //API seems to have changed. Error strings are on top level of object
                //item->setText(5, doc.object()["0"].toObject()[notr("error")].toString());
                item->setText(5, doc.object()[notr("error")].toString());
            }*/

            errorItems.append(item);
        }

        if (cancelled) {
            break;
        }
    }

    emit progress(1000, tr("Signing out..."));
    vfsync->signout();

    if (counter != successCounter) {
        emit finished(true, QString(tr("%1 out of %2 flights could not be uploaded.")).arg(counter - successCounter).arg(counter), errorItems);
    } else {
        emit finished(false, tr("Upload completed, %1 flights have been transmitted.").arg(counter), errorItems);
    }

}
