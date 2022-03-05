#ifndef VEREINSFLIEGERFLIGHT_H
#define VEREINSFLIEGERFLIGHT_H

#include <QString>
#include <QDateTime>
#include "src/model/Flight.h"
#include "src/db/DbManager.h"

class VereinsfliegerFlight
{
private:
    void copyFrom(const VereinsfliegerFlight& f);
public:
    VereinsfliegerFlight();

    VereinsfliegerFlight(const Flight& flight, DbManager* dbManager);
    VereinsfliegerFlight(const VereinsfliegerFlight& f);
    VereinsfliegerFlight& operator=(const VereinsfliegerFlight&);

    void readJson(const QJsonObject &json);
    void writeJson(QJsonObject &json) const;

    qlonglong id;
    qlonglong vfid;
    QString callsign;
    QString pilotname;          // Nachname, Vorname
    QString attendantname;      // Nachname, Vorname
    QString starttype;          // E=Eigenstart, W=Windenstart, F=F-Schlepp
    QDateTime departuretime;    // yyyy-mm-dd HH:MM
    QString departurelocation;
    QDateTime arrivaltime;      // yyyy-mm-dd HH:MM
    QString arrivallocation;
    int landingcount;
    int chargemode;             // 1=Keine, 2=Pilot, 3=Begleiter, 4=Gast, 5=Pilot+Begleiter
    int ftid;                   // 10=Standard oder 8=Schulflug (mehr bei den Stammdaten/Flugarten)
    QString comment;
    int towtime;
    int towheight;
    QString towcallsign;
    QString towpilotname;       // Nachname, Vorname

};

#endif // VEREINSFLIEGERFLIGHT_H
