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

struct VereinsfliegerFlight {
    qlonglong vfid;
    QString callsign;
    QString pilotname;
    QString attendantname;
    QString starttype; // E,W,F
    QDateTime departuretime;
    QString departurelocation;
    QDateTime arrivaltime;
    QString arrivallocation;
    int landingcount;
    QString ftid; // Flugart als Zahl
};

class VereinsfliegerSync : public QObject
{
    Q_OBJECT
public:
    explicit VereinsfliegerSync(QNetworkAccessManager* man, QObject *parent = 0);

    int retrieveAccesstoken();
    int signin(QString user, QString pass, int cid = 0);
    int signout();

    int getuser();
    int addflight(VereinsfliegerFlight& flight);

private:
    QNetworkAccessManager* manager;

    const QString baseUrl = "https://vereinsflieger.de/interface/rest/";
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
