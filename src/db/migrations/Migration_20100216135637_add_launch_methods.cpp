#include "Migration_20100216135637_add_launch_methods.h"

#include <iostream>

#include <QFile>
#include <QTextStream>

#include "src/model/LaunchMethod.h"
#include "src/util/qString.h"
#include "src/util/environment.h"
#include "src/text.h"

REGISTER_MIGRATION (20100216135637, add_launch_methods)

Migration_20100216135637_add_launch_methods::Migration_20100216135637_add_launch_methods (Interface &interface):
	Migration (interface)
{
}

Migration_20100216135637_add_launch_methods::~Migration_20100216135637_add_launch_methods ()
{
}

void Migration_20100216135637_add_launch_methods::up ()
{
	createTable  ("launch_methods"); // Creates the id column
	addColumn ("launch_methods", "name"                 , dataTypeString    ());
	addColumn ("launch_methods", "short_name"           , dataTypeString    ());
	addColumn ("launch_methods", "log_string"           , dataTypeString    ());
	addColumn ("launch_methods", "keyboard_shortcut"    , dataTypeCharacter ());
	// Use a string type because SQLite does not support enums
	addColumn ("launch_methods", "type"                 , dataTypeString    ());
	addColumn ("launch_methods", "towplane_registration", dataTypeString    ());
	addColumn ("launch_methods", "person_required"      , dataTypeBoolean   ());
	addColumn ("launch_methods", "comments"             , dataTypeString    ());

	try
	{
		transaction ();

		foreach (LaunchMethod launchMethod, readConfiguredLaunchMethods ())
		{
			// Don't use the methods of Database or LaunchMethod - they use the
			// current schema, we need to use the schema after this migration.
			std::cout << "Importing launch method: " << launchMethod.toString () << std::endl;

			Query query (
				"INSERT INTO launch_methods"
				"(id,name,short_name,log_string,keyboard_shortcut,type,towplane_registration,person_required,comments)"
				"values (?,?,?,?,?,?,?,?,?)"
				);

			query.bind (launchMethod.getId ());
			query.bind (launchMethod.name);
			query.bind (launchMethod.shortName);
			query.bind (launchMethod.logString);
			query.bind (launchMethod.keyboardShortcut);
			query.bind (LaunchMethod::typeToDb (launchMethod.type));
			query.bind (launchMethod.towplaneRegistration);
			query.bind (launchMethod.personRequired);
			query.bind (launchMethod.comments);

			executeQuery (query);
		}

		commit ();
	}
	catch (...)
	{
		rollback ();
		throw;
	}
}

void Migration_20100216135637_add_launch_methods::down ()
{
	dropTable ("launch_methods");
}

const QString default_home_config_filename=".startkladde.conf";
const QString default_local_config_fielname="startkladde.conf";

QString Migration_20100216135637_add_launch_methods::effectiveConfigFileName ()
{
	QString homeConfigFileName=getEnvironmentVariable ("HOME")+"/"+default_home_config_filename;
	if (QFile::exists (homeConfigFileName)) return homeConfigFileName;

	if (QFile::exists (default_local_config_fielname))
		return default_local_config_fielname;

	return "";
}

// Old code from Options class
QList<LaunchMethod> Migration_20100216135637_add_launch_methods::readConfiguredLaunchMethods ()
{
	QList<LaunchMethod> launchMethods;

	QString filename=effectiveConfigFileName ();
	if (filename.isEmpty ()) return launchMethods;

	QFile configFile (filename);

	if (!configFile.open (QIODevice::ReadOnly))
	{
		std::cout << "Configuration file " << filename << " could not be opened" << std::endl;
		return launchMethods;
	}

	QTextStream stream (&configFile);

	while (!stream.atEnd ())
	{
		std::string line=q2std (stream.readLine ().trimmed());

		if (line.length ()>0 && line[0]!='#')
		{
			QString key, value;
			// We use std::string here because it has find_first_of and find_first_not_of
			std::string::size_type pos=line.find_first_of (q2std (whitespace)); // TODO conversion is done for every line

			if (pos!=std::string::npos)
			{
				key=std2q (line.substr (0, pos));
				pos=line.find_first_not_of (q2std (whitespace), pos); // TODO conversion is done for every line
				value=std2q (line.substr (pos)).trimmed();
			}
			else
			{
				key=std2q (line);
			}

			if (key=="startart")
			{
				LaunchMethod sa=LaunchMethod::parseConfigLine (value);
				if (idInvalid (sa.getId ()))
					std::cerr << "Error: launch method with invalid ID " << sa.getId () << " specified." << std::endl;
				else
					launchMethods.append (sa);
			}

		}
	}

	configFile.close ();

	return launchMethods;
}
