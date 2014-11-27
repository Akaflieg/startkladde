#ifndef DATABASEINFO_H_
#define DATABASEINFO_H_

#include <QString>

class QSettings;

class DatabaseInfo
{
	public:
		DatabaseInfo ();
        DatabaseInfo (QString server, QString username, QString password, QString database);
		DatabaseInfo (QSettings &settings);
		virtual ~DatabaseInfo ();

		virtual QString toString () const;
		virtual operator QString () const;
		virtual QString serverText () const;

        virtual void load (QSettings &settings, QString suffix = "");
        virtual void save (QSettings &settings, QString suffix = "");

		virtual int effectivePort () const { return defaultPort ? 3306 : port; }

		virtual bool different (const DatabaseInfo &other);

		// Update: toString, serverText, load, save, different
		QString server;
		bool defaultPort;
		int port;
		QString username;
		QString password;
		QString database;
};

#endif
