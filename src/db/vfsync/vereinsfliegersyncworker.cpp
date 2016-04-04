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
    if (vfsync->retrieveAccesstoken() != 0)
    {
        emit finished(true, tr("Connection to Vereinsflieger could not be initiated."));
        return;
    }

    if (cancelled)
    {
        emit finished(true, tr("Operation cancelled by user."));
        return;
    }

    emit progress(0, tr("Signing in..."));
    if (vfsync->signin(user, pass) != 0)
    {
        emit finished(true, tr("Failed to sign in on Vereinsflieger."));
        return;
    }

    if (cancelled)
    {
        emit finished(true, tr("Operation cancelled by user."));
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
        if (vfsync->addflight(f) == 0)
        {
            flight.setVfId(f.vfid);
            dbManager->getDb().updateObject<Flight>(flight);
            successCounter++;
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
        emit finished(true, QString(tr("%1 out of %2 flights could not be uploaded.")).arg(counter - successCounter).arg(counter));
    } else
    {
        emit finished(false, tr("Upload completed, %1 flights have been transmitted.").arg(counter));
    }

}

VereinsfliegerFlight VereinsfliegerSyncWorker::convertFlight(Flight& flight)
{
    VereinsfliegerFlight result;

    if (flight.getPlaneId() != 0)
    {
        Plane p = dbManager->getCache().getObject<Plane>(flight.getPlaneId());
        result.callsign = p.registration;
    }

    if (flight.getPilotId() != 0)
    {
        Person p = dbManager->getCache().getObject<Person>(flight.getPilotId());
        result.pilotname = p.lastName + ", " + p.firstName;
    }

    if (flight.getCopilotId() != 0)
    {
        Person p = dbManager->getCache().getObject<Person>(flight.getCopilotId());
        result.attendantname = p.lastName + ", " + p.firstName;
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
    }


    result.departuretime = flight.getDepartureTime();
    result.departurelocation = flight.getDepartureLocation();
    result.arrivaltime = flight.getLandingTime();
    result.arrivallocation = flight.getLandingLocation();
    result.landingcount = flight.getNumLandings();
    result.comment = flight.getComments();

    switch(flight.getType())
    {
        case Flight::typeTraining1:     result.ftid = 8;    result.chargemode = 2; break;
        case Flight::typeTraining2:     result.ftid = 8;    result.chargemode = 2; break;
        case Flight::typeGuestPrivate:  result.ftid = 14;   result.chargemode = 2; break;
        case Flight::typeGuestExternal: result.ftid = 13;   result.chargemode = 4; break;
        default:                        result.ftid = 10;   result.chargemode = 2;
    }

    return result;
}
