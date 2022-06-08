#include "vereinsfliegersync.h"

VereinsfliegerSync::VereinsfliegerSync(QNetworkAccessManager* man, QObject *parent) : QObject(parent)
{
    baseUrl = "https://vereinsflieger.de/interface/rest/";

    manager = man;
    accesstoken = QString();
    signedIn = QString();
}

int VereinsfliegerSync::retrieveAccesstoken()
{
    ReplyData reply = get("auth/accesstoken");
    accesstoken = reply.replyValues["accesstoken"].toString();
    qDebug() << reply.replyString;

    if (reply.cancelled) {
        return 2;
    } else if (!accesstoken.isEmpty()) {
        return 0;
    } else {
        return 1;
    }
}

int VereinsfliegerSync::signin(QString user, QString pass, int cid, QString appkey)
{
    QMap<QString, QString> args;
    args.insert("username", user);
    args.insert("password", md5(pass));
    args.insert("appkey", appkey);
    args.insert("cid", QString("%1").arg(cid));
    ReplyData reply = post("auth/signin", args);
    qDebug() << reply.replyString;

    if (reply.cancelled) {
        return 2;
    } else if (reply.httpStatus == 200) {
        return 0;
    } else {
        //Already logged in
        if (reply.replyValues.value("error") == "Unknown Resource") {
            return 0;
        }
        return 1;
    }
}

void VereinsfliegerSync::addflight(VereinsfliegerFlight& flight)
{
    QMap<QString, QString> args;

    args.insert("callsign", flight.callsign);
    if (!flight.pilotname.isEmpty())        args.insert("pilotname", flight.pilotname);
    if (!flight.attendantname.isEmpty())    args.insert("attendantname", flight.attendantname);
    if (!flight.starttype.isEmpty())        args.insert("starttype", flight.starttype);
    if (!flight.departurelocation.isEmpty())args.insert("departurelocation", flight.departurelocation);
    if (!flight.arrivallocation.isEmpty())  args.insert("arrivallocation", flight.arrivallocation);
    if (flight.landingcount != 1)           args.insert("landingcount", QString::number(flight.landingcount));
    if (flight.departuretime.isValid())     args.insert("departuretime", flight.departuretime.toString("yyyy-MM-dd HH:mm"));
    if (flight.arrivaltime.isValid())       args.insert("arrivaltime", flight.arrivaltime.toString("yyyy-MM-dd HH:mm"));
    if (!flight.comment.isEmpty())          args.insert("comment", flight.comment);
    if (flight.towtime > 0)                 args.insert("towtime", QString::number(flight.towtime));
    if (flight.towheight > 0)               args.insert("towheight", QString::number(flight.towheight));
    if (!flight.towcallsign.isEmpty())      args.insert("towcallsign", flight.towcallsign);
    if (!flight.towpilotname.isEmpty())     args.insert("towpilotname", flight.towpilotname);
    args.insert("ftid", flight.ftid <= 0 ? "10" : QString::number(flight.ftid));
    args.insert("chargemode", flight.chargemode <= 0 ? "2" : QString::number(flight.chargemode));

    ReplyData reply = post("flight/add", args);
    qDebug() << reply.replyString;

    if (reply.cancelled) {
        throw VfSyncException(true, "", 0);
    } else if (reply.httpStatus == 200) {
        flight.vfid = reply.replyValues.value("flid").toLongLong();
    } else {
        throw VfSyncException(false, reply.replyString, reply.httpStatus);
    }
}

void VereinsfliegerSync::editflight (VereinsfliegerFlight& flight)
{
    QMap<QString, QString> args;

    // when we edit flight, transfer all data to make sure all fields are overwritten
    // example: copilot has been removed
    args.insert("callsign", flight.callsign);
    args.insert("pilotname", flight.pilotname);
    args.insert("attendantname", flight.attendantname);
    args.insert("starttype", flight.starttype);
    args.insert("departurelocation", flight.departurelocation);
    args.insert("arrivallocation", flight.arrivallocation);
    args.insert("landingcount", QString::number(flight.landingcount));
    if (flight.departuretime.isValid())     args.insert("departuretime", flight.departuretime.toString("yyyy-MM-dd HH:mm"));
    if (flight.arrivaltime.isValid())       args.insert("arrivaltime", flight.arrivaltime.toString("yyyy-MM-dd HH:mm"));
    args.insert("comment", flight.comment);
    if (flight.towtime > 0)                 
        args.insert("towtime", QString::number(flight.towtime));
    else
        args.insert("towtime", "0");
    if (flight.towheight > 0)               
        args.insert("towheight", QString::number(flight.towheight));
    else
        args.insert("towheight", "0");
    args.insert("towcallsign", flight.towcallsign);
    args.insert("towpilotname", flight.towpilotname);
    
    args.insert("ftid", flight.ftid <= 0 ? "10" : QString::number(flight.ftid));
    args.insert("chargemode", flight.chargemode <= 0 ? "2" : QString::number(flight.chargemode));

    qDebug () << "VereinsfliegerSync::editflight: " << QString("flight/edit/%1").arg(flight.vfid);
    ReplyData reply = put(QString("flight/edit/%1").arg(flight.vfid), args);
    qDebug() << "reply: " << reply.replyString;

    if (reply.cancelled) {
        throw VfSyncException(true, "", 0);
    } else if (reply.httpStatus == 200) {
        flight.vfid = reply.replyValues.value("flid").toLongLong();
    } else {
        throw VfSyncException(false, reply.replyString, reply.httpStatus);
    }
}

int VereinsfliegerSync::signout()
{
    QMap<QString,QString> args;
    ReplyData reply = del("auth/signout", args);
    qDebug() << reply.replyString;
    if (reply.cancelled) {
        return 2;
    } else if (reply.httpStatus == 200) {
        foreach (QNetworkCookie cookie, manager->cookieJar()->cookiesForUrl(QUrl("https://vereinsflieger.de/")))
        {
            manager->cookieJar()->deleteCookie(cookie);
        }

        return 0;
    } else {
        return 1;
    }
}

int VereinsfliegerSync::getuser()
{
    QMap<QString,QString> args;
    ReplyData reply = post("auth/getuser", args);
    qDebug() << reply.replyString;
    if (reply.cancelled) {
        return 2;
    } else if (reply.httpStatus == 200) {
        return 0;
    } else {
        return 1;
    }
}

ReplyData VereinsfliegerSync::get(QString service)
{
    QString urlString = baseUrl + service;
    QUrl url = QUrl(urlString);
    QNetworkRequest request;

    request.setUrl(url);

    QNetworkReply *reply = NULL;
    reply = manager->get(request);

    QEventLoop loop;
    connect(this, SIGNAL(cancelled()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();

    ReplyData result;

    if (reply->isFinished()) {
        result.replyString = reply->readAll();
        result.replyValues = extractReturnValues(result.replyString);
        result.httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        result.cancelled = false;
    } else {
        result.cancelled = true;
    }

    reply->deleteLater();
    return result;
}

ReplyData VereinsfliegerSync::post(QString service, QMap<QString,QString>& args)
{
    QUrl url = QUrl(baseUrl + service);
    QNetworkRequest request;

    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("accesstoken", accesstoken);

    foreach (QString key, args.keys())
    {
        if (key != "accesstoken")
        {
            params.addQueryItem(key, args.value(key));
        }
    }

    QNetworkReply *reply = NULL;
    reply = manager->post(request, params.query().toUtf8());

    QEventLoop loop;
    connect(this, SIGNAL(cancelled()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();

    ReplyData result;
    if (reply->isFinished()) {
        result.replyString = reply->readAll();
        result.replyValues = extractReturnValues(result.replyString);
        result.httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        result.cancelled = false;
    } else {
        result.cancelled = true;
    }

    reply->deleteLater();
    return result;
}

ReplyData VereinsfliegerSync::put(QString service, QMap<QString,QString>& args)
{
    QUrl url = QUrl(baseUrl + service);
    QNetworkRequest request;

    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("accesstoken", accesstoken);

    foreach (QString key, args.keys())
    {
        if (key != "accesstoken")
        {
            params.addQueryItem(key, args.value(key));
        }
    }

    QNetworkReply *reply = NULL;
    qDebug () << "VereinsfliegerSync::put: request=" << request.url().toDisplayString() << "; query=" << params.query().toUtf8(); 
    reply = manager->put(request, params.query().toUtf8());

    QEventLoop loop;
    connect(this, SIGNAL(cancelled()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();

    ReplyData result;
    if (reply->isFinished()) {
        result.replyString = reply->readAll();
        result.replyValues = extractReturnValues(result.replyString);
        result.httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        result.cancelled = false;
    } else {
        result.cancelled = true;
    }

    reply->deleteLater();
    return result;
}

ReplyData VereinsfliegerSync::del(QString service, QMap<QString,QString>& args)
{
    QUrl url = QUrl(baseUrl + service);
    QNetworkRequest request;

    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("accesstoken", accesstoken);

    foreach (QString key, args.keys())
    {
        if (key != "accesstoken")
        {
            params.addQueryItem(key, args.value(key));
        }
    }

    QString delStr = "DELETE";
    QByteArray data = params.query().toUtf8();

    QBuffer *buffer = new QBuffer;
    buffer->setData(data);
    buffer->open(QIODevice::ReadOnly);
    QNetworkReply *reply = NULL;
    //reply = manager->deleteResource(request);
    reply = manager->sendCustomRequest(request, delStr.toLatin1(), buffer);

    QEventLoop loop;
    connect(this, SIGNAL(cancelled()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();

    ReplyData result;
    if (reply->isFinished()) {
        result.replyString = reply->readAll();
        result.replyValues = extractReturnValues(result.replyString);
        result.httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        result.cancelled = false;
    } else {
        result.cancelled = true;
    }

    reply->deleteLater();
    return result;
}

void VereinsfliegerSync::cancel()
{
    emit cancelled();
}

QMap<QString,QVariant> VereinsfliegerSync::extractReturnValues(QByteArray& data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QMap<QString,QVariant> result = doc.object().toVariantMap();
    return result;
}

QString VereinsfliegerSync::md5(QString str)
{
    return QString(QCryptographicHash::hash(str.toUtf8(),QCryptographicHash::Md5).toHex());
}
