#include "vereinsfliegersyncworker.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/model/LaunchMethod.h"

VereinsfliegerSyncWorker::VereinsfliegerSyncWorker(DbManager* _dbManager, QString _user, QString _pass, QObject* parent) : QObject(parent)
{
    dbManager = _dbManager;
    user = _user;
    pass = _pass;
    vfsync = new VereinsfliegerSync(Network::getNetworkAccessManager(), this);
    cancelled = false;
}

VereinsfliegerSyncWorker::~VereinsfliegerSyncWorker()
{
    delete vfsync;
}

void VereinsfliegerSyncWorker::cancel()
{
    vfsync->cancel();
    cancelled = true;
}

void VereinsfliegerSyncWorker::sync()
{
    errorItems.clear();

    if (vfsync->retrieveAccesstoken() != 0)
    {
        emit finished(true, tr("Connection to Vereinsflieger could not be initiated."), errorItems);
        return;
    }

    if (cancelled)
    {
        emit finished(true, tr("Operation cancelled by user."), errorItems);
        return;
    }

    emit progress(0, tr("Signing in..."));

    //TODO: Vereins-CIDS
    QList<int> cidList;
    cidList << 370 << 250;

    bool signedIn = false;
    foreach (int cid, cidList)
    {
        if (!signedIn)
        {
            signedIn = vfsync->signin(user, pass, cid) == 0;
        }
    }

    if (!signedIn)
    {
        emit finished(true, tr("Failed to sign in on Vereinsflieger."), errorItems);
        return;
    }

    if (cancelled)
    {
        emit finished(true, tr("Operation cancelled by user."), errorItems);
        return;
    }

    emit progress(0, tr("Fetching flights..."));
    QList<Flight> flights = dbManager->getDb().getFlightsUnimportedVF();

    int counter = 0;
    int successCounter = 0;
    foreach(Flight flight, flights)
    {
        counter++;
        emit progress(((double) counter / (double) flights.size())*1000, QString(tr("Uploading flight %1 of %2...")).arg(counter).arg(flights.size()));

        VereinsfliegerFlight f = convertFlight(flight);

        try {
            vfsync->addflight(f);
            flight.setVfId(f.vfid);
            dbManager->getDb().updateObject<Flight>(flight);
            successCounter++;
        } catch (VfSyncException ex) {
            QTreeWidgetItem* item = new QTreeWidgetItem();
            item->setText(0, f.pilotname);
            item->setText(1, f.attendantname);
            item->setText(2, f.departuretime.toString("dd.MM.yyyy"));
            item->setText(3, f.departuretime.toString("HH:mm"));
            item->setText(4, f.arrivaltime.toString("HH:mm"));

            QJsonDocument doc = QJsonDocument::fromJson(ex.replyString.toUtf8());

            if (ex.cancelled) {
                item->setText(5, tr("Connection cancelled!"));
            } else {
                item->setText(5, doc.object()[notr("error")].toString());
            }

            errorItems.append(item);
        }

        if (cancelled)
        {
            break;
        }

    }

    emit progress(1000, tr("Signing out..."));
    vfsync->signout();

    if (counter != successCounter)
    {
        emit finished(true, QString(tr("%1 out of %2 flights could not be uploaded.")).arg(counter - successCounter).arg(counter), errorItems);
    } else
    {
        emit finished(false, tr("Upload completed, %1 flights have been transmitted.").arg(counter), errorItems);
    }

}

VereinsfliegerFlight VereinsfliegerSyncWorker::convertFlight(Flight& flight)
{
    VereinsfliegerFlight result;

    if (flight.getPlaneId() != 0)
    {
        Plane plane = dbManager->getCache().getObject<Plane>(flight.getPlaneId());
        result.callsign = plane.registration.trimmed();
    }

    if (flight.getPilotId() != 0)
    {
        Person pilot = dbManager->getCache().getObject<Person>(flight.getPilotId());
        result.pilotname = pilot.lastName.trimmed() + ", " + pilot.firstName.trimmed();
    }

    if (flight.getCopilotId() != 0)
    {
        Person copilot = dbManager->getCache().getObject<Person>(flight.getCopilotId());
        result.attendantname = copilot.lastName.trimmed() + ", " + copilot.firstName.trimmed();
    }

    if (flight.getLaunchMethodId() != 0)
    {
        LaunchMethod lm = dbManager->getCache().getObject<LaunchMethod>(flight.getLaunchMethodId());
        switch (lm.type)
        {
            case LaunchMethod::typeWinch: result.starttype = notr("W"); break;
            case LaunchMethod::typeSelf: result.starttype = notr("E"); break;
            case LaunchMethod::typeAirtow: result.starttype = notr("F"); break;
            default: result.starttype = notr("W");
        }

        if (lm.type == LaunchMethod::typeAirtow) {
            if (flight.getTowpilotId() != 0) {
                Person towpilot = dbManager->getCache().getObject<Person>(flight.getTowpilotId());
                result.towpilotname = towpilot.lastName.trimmed() + ", " + towpilot.firstName.trimmed();
            }

            if (flight.getTowplaneId() != 0) {
                Plane towplane = dbManager->getCache().getObject<Plane>(flight.getTowplaneId());
                result.towcallsign = towplane.registration;
            }

            result.towtime = flight.getDepartureTime().msecsTo(flight.getTowflightLandingTime()) / 60000;

            QRegExp rx ("(schlepphöhe|schlepphoehe|hoehe|höhe|auf|ausklinkhöhe)?:?\\W*([0-9]+)\\W*(m|meter|mtr)");
            if (rx.indexIn(flight.getComments().toLower()) != -1) {
                result.towheight = rx.cap(2).toInt();
            } else {
                result.towheight = 0;
            }

        } else {
            result.towheight = 0;
            result.towtime = 0;
            result.towcallsign = QString();
            result.towpilotname = QString();
        }
    }

    result.departuretime = flight.getDepartureTime();
    result.departurelocation = flight.getDepartureLocation();
    result.arrivaltime = flight.getLandingTime();
    result.arrivallocation = flight.getLandingLocation();
    result.landingcount = flight.getNumLandings();
    result.comment = flight.getComments();

    // Abrechnungsmodus
    // 1=Keine, 2=Pilot, 3=Begleiter, 4=Gast, 5=Pilot+Begleiter

    switch (flight.getType())
    {
        case Flight::typeTraining1:     result.ftid = 8;    result.chargemode = 2; break;
        case Flight::typeTraining2:     result.ftid = 8;    result.chargemode = 2; break;
        case Flight::typeGuestPrivate:  result.ftid = 14;   result.chargemode = 2; break;
        case Flight::typeGuestExternal: result.ftid = 13;   result.chargemode = 4; break;
        default:                        result.ftid = 10;   result.chargemode = 2;
    }

    // TODO
    // Wenn Flugzeug CT und Pilot AFV -> Dann nicht abrechnen
    if (flight.getPlaneId() != 0 && flight.getPilotId() != 0)
    {
        Plane plane = dbManager->getCache().getObject<Plane>(flight.getPlaneId());
        Person pilot = dbManager->getCache().getObject<Person>(flight.getPilotId());

        if (plane.registration.trimmed() == "D-1877" && pilot.club.trimmed() == "AFV")
        {
            result.chargemode = 1;
        }
    }

    return result;
}
