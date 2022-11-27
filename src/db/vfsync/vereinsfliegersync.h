#ifndef VEREINSFLIEGERSYNC_H
#define VEREINSFLIEGERSYNC_H

#include <QObject>
#include <QtNetwork>
#include <optional>
#include "vereinsfliegerflight.h"

struct ReplyData
{
    QByteArray replyString;
    QMap<QString,QVariant> replyValues;
    int httpStatus;
    bool cancelled;
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

    VfSyncException(QString replyString, int httpStatus): replyString(replyString), httpStatus(httpStatus) { }
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
    void editflight(VereinsfliegerFlight& flight);

private:
    QNetworkAccessManager* manager;

    QString baseUrl;
    ReplyData post(QString url, QMap<QString,QString>& args);
    ReplyData put(QString url, QMap<QString,QString>& args);
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
