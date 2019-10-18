#ifndef VEREINSFLIEGERSYNC_H
#define VEREINSFLIEGERSYNC_H

#include <QObject>
#include <QtNetwork>

struct ReplyData
{
    QByteArray replyString;
    QMap<QString,QVariant> replyValues;
    int httpStatus;
    bool cancelled;
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
    int ftid;                   // Flugart als Zahl
    QString comment;
    int towtime;
    int towheight;
    QString towcallsign;
    QString towpilotname;
};

struct VfSyncException
{
    bool cancelled;
    QString replyString;
    int httpStatus;

    VfSyncException(bool cancelled, QString replyString, int httpStatus):
        cancelled(cancelled), replyString(replyString), httpStatus(httpStatus) { }
};

class VereinsfliegerSync : public QObject
{
    Q_OBJECT
public:
    explicit VereinsfliegerSync(QNetworkAccessManager* man, QObject *parent = 0);

    int retrieveAccesstoken();
    int signin(QString user, QString pass, int cid, QString appkey);
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

    QString accesstoken;
    QString signedIn;

signals:
    void cancelled();

public slots:
    void cancel();
};

#endif // VEREINSFLIEGERSYNC_H
