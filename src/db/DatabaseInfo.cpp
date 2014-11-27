#include "DatabaseInfo.h"

#include <QSettings>

#include <iostream>

#include "src/i18n/notr.h"

DatabaseInfo::DatabaseInfo ():
	defaultPort (true), port (0)
{
}

DatabaseInfo::DatabaseInfo (QString srv, QString user, QString pass, QString db) :
    defaultPort(true), port(0)
{
    server = srv;
    username = user;
    password = pass;
    database = db;
}

DatabaseInfo::~DatabaseInfo ()
{
}

DatabaseInfo::DatabaseInfo (QSettings &settings):
	defaultPort (true), port (0)
{
	load (settings);
}


QString DatabaseInfo::toString () const
{
	return qnotr ("%1@%2:%3").arg (username, server, database);
}

DatabaseInfo::operator QString () const
{
	return toString ();
}

QString DatabaseInfo::serverText () const
{
	if (defaultPort)
		return server;
	else
		return qnotr ("%1:%2").arg (server).arg (port);
}

void DatabaseInfo::load (QSettings &settings, QString suffix)
{
    if (suffix.isEmpty())
        server     =settings.value (notr (QString("server%1").arg(suffix))     , notr ("localhost")  ).toString ();
    else
        server     =settings.value (notr (QString("server%1").arg(suffix))     , notr ("remotehost")  ).toString ();

    defaultPort=settings.value (notr (QString("defaultPort%1").arg(suffix)), true                ).toBool   ();
    port       =settings.value (notr (QString("port%1").arg(suffix))       , 3306                ).toInt    ();
    username   =settings.value (notr (QString("username%1").arg(suffix))   , notr ("startkladde")).toString ();
    password   =settings.value (notr (QString("password%1").arg(suffix))   , notr ("sk")         ).toString ();
    database   =settings.value (notr (QString("database%1").arg(suffix))   , notr ("startkladde")).toString ();
}

void DatabaseInfo::save (QSettings &settings, QString suffix)
{
    settings.setValue (notr (QString("server%1").arg(suffix))     , server     );
    settings.setValue (notr (QString("defaultPort%1").arg(suffix)), defaultPort);
    settings.setValue (notr (QString("port%1").arg(suffix))       , port       );
    settings.setValue (notr (QString("username%1").arg(suffix))   , username   );
    settings.setValue (notr (QString("password%1").arg(suffix))   , password   );
    settings.setValue (notr (QString("database%1").arg(suffix))   , database   );
}

/**
 * Determines whether this databaseInfo refers to a different database than
 * another one
 *
 * The databases can be identical (i. e. this method returns false) even if
 * some values are different, for example, if they are not active because they
 * belong to a different database type than selected.
 *
 * @param other
 * @return
 */
bool DatabaseInfo::different (const DatabaseInfo &other)
{
	if (server          !=other.server          ) return true;
	if (effectivePort ()!=other.effectivePort ()) return true;
	if (username        !=other.username        ) return true;
	if (password        !=other.password        ) return true;
	if (database        !=other.database        ) return true;

	return false;
}
