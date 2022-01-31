#ifndef VEREINSFLIEGERSYNC_H
#define VEREINSFLIEGERSYNC_H

#include <QObject>
#include <QtNetwork>
#include <optional>

struct ReplyData
{
    QByteArray replyString;
    QMap<QString, QVariant> replyValues;
    int httpStatus = 0;
    bool cancelled = false;
};

struct VereinsfliegerFlight
{
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

struct VfCredentials
{
    QString appkey;
    QString user;
    QString pass;
    QList<int> cidList;
};

struct VfSyncException
{
    QString replyString;
    int httpStatus;

    VfSyncException(QString replyString, int httpStatus):
        replyString(replyString), httpStatus(httpStatus) { }
};

class VereinsfliegerSync : public QObject
{
    Q_OBJECT
public:
    explicit VereinsfliegerSync(QNetworkAccessManager* man, VfCredentials creds, QObject *parent = 0);

    int retrieveAccesstoken();
    bool signin();

    int signout();

    int getuser();
    void addflight(VereinsfliegerFlight& flight);

private:
    QNetworkAccessManager* manager;

    QString baseUrl;
    ReplyData post(QString url, QMap<QString,QString>& args);
    ReplyData del(QString url, QMap<QString,QString>& args);
    ReplyData get(QString url);

    QMap<QString,QVariant> extractReturnValues(QByteArray&);
    QString md5(QString);
    bool signinWithCid(std::optional<int> cid);

    VfCredentials creds;
    QString accesstoken;

signals:
    void cancelled();

public slots:
    void cancel();
};

#endif // VEREINSFLIEGERSYNC_H
