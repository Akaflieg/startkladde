#include "vereinsfliegerflight.h"

#include "src/model/Plane.h"
#include "src/model/Person.h"

VereinsfliegerFlight::VereinsfliegerFlight()
{
    vfid = 0;
    landingcount = 0;
    chargemode = 0;
    ftid = 0;
    towheight = 0;
    towtime = 0;
}

VereinsfliegerFlight::VereinsfliegerFlight(const VereinsfliegerFlight& f) {
    copyFrom(f);
}

VereinsfliegerFlight& VereinsfliegerFlight::operator=(const VereinsfliegerFlight& f) {
    copyFrom(f);
    return *this;
}

void VereinsfliegerFlight::copyFrom(const VereinsfliegerFlight& f) {
    this->id = f.id;
    this->vfid = f.vfid;
    this->callsign = f.callsign;
    this->pilotname = f.pilotname;
    this->attendantname = f.attendantname;
    this->starttype = f.starttype;
    this->departuretime = f.departuretime;
    this->departurelocation = f.departurelocation;
    this->arrivaltime = f.arrivaltime;
    this->arrivallocation = f.arrivallocation;
    this->landingcount = f.landingcount;
    this->chargemode = f.chargemode;
    this->ftid = f.ftid;
    this->comment = f.comment;
    this->towtime = f.towtime;
    this->towheight = f.towheight;
    this->towcallsign = f.towcallsign;
    this->towpilotname = f.towpilotname;
}

VereinsfliegerFlight::VereinsfliegerFlight(const Flight& flight, DbManager* dbManager) {
    VereinsfliegerFlight& result = *this;
    result.id = flight.getId();
    result.vfid = flight.getVfId();

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

            // Versuche, die Schlepphöhe aus dem Kommentartext herauszuholen
            QRegExp rx ("(schlepphöhe|schlepphoehe|hoehe|höhe|auf|ausklinkhöhe|ausklinkhoehe)?:?\\W*([0-9]+)\\W*(m|meter|mtr)");
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
    // 1=keine, 2=Pilot, 3=Begleiter,4=Gast, 5=Pilot+Begleiter, 7=Anderes Mitglied
    // Anderes Mitglied kann mit 'uidcharge' angegeben werden
    switch (flight.getType())
    {
        case Flight::typeTraining1:     result.ftid = 8;    result.chargemode = 2; break;
        case Flight::typeTraining2:     result.ftid = 8;    result.chargemode = 2; break;
        case Flight::typeGuestPrivate:  result.ftid = 14;   result.chargemode = 2; break;
        case Flight::typeGuestExternal: result.ftid = 13;   result.chargemode = 4; break;
        default:                        result.ftid = 10;   result.chargemode = 2;
    }
}

void VereinsfliegerFlight::readJson(const QJsonObject &json) {
    id = json["id"].toInt();
    vfid = json["vfid"].toInt();
    callsign = json["callsign"].toString();
    pilotname = json["pilotname"].toString();
    attendantname = json["attendantname"].toString();
    starttype = json["starttype"].toString();
    departuretime = QDateTime::fromString(json["departuretime"].toString(), Qt::ISODate);
    departurelocation = json["departurelocation"].toString();
    arrivaltime = QDateTime::fromString(json["arrivaltime"].toString(), Qt::ISODate);
    arrivallocation = json["arrivallocation"].toString();
    landingcount = json["landingcount"].toInt();
    chargemode = json["chargemode"].toInt();
    ftid = json["ftid"].toInt();
    comment = json["comment"].toString();
    towtime = json["towtime"].toInt();
    towheight = json["towheight"].toInt();
    towcallsign = json["towcallsign"].toString();
    towpilotname = json["towpilotname"].toString();
}

void VereinsfliegerFlight::writeJson(QJsonObject &json) const {
    json["id"] = id;
    json["vfid"] = vfid;
    json["callsign"] = callsign;
    json["pilotname"] = pilotname;
    json["attendantname"] = attendantname;
    json["starttype"] = starttype;
    json["departuretime"] = departuretime.toString(Qt::ISODate);
    json["departurelocation"] = departurelocation;
    json["arrivaltime"] = arrivaltime.toString(Qt::ISODate);
    json["arrivallocation"] = arrivallocation;
    json["landingcount"] = landingcount;
    json["chargemode"] = chargemode;
    json["ftid"] = ftid;
    json["comment"] = comment;
    json["towtime"] = towtime;
    json["towheight"] = towheight;
    json["towcallsign"] = towcallsign;
    json["towpilotname"] = towpilotname;
}
