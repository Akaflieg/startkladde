#include "LaunchMethod.h"

#include <cassert>

#include <QVariant>
#include <QApplication>

#include "src/logging/messages.h"
#include "src/util/bool.h"
#include "src/db/result/Result.h"
#include "src/db/Query.h"
#include "src/util/qString.h"
#include "src/i18n/notr.h"
#include "src/text.h"


// ******************
// ** Construction **
// ******************

LaunchMethod::LaunchMethod ():
	Entity ()
{
	initialize ();
}

LaunchMethod::LaunchMethod (dbId id):
	Entity (id)
{
	initialize ();
}

// Class management
void LaunchMethod::initialize ()
{
	towplaneRegistration="";
	keyboardShortcut="";
	name=notr ("---");
	shortName=notr ("-");
	logString=notr ("-");
	personRequired=true;
	type=typeOther;
}

LaunchMethod LaunchMethod::parseConfigLine (QString line)
{
	LaunchMethod launchMethod;

	// Split the line and simplify whitespace
	QStringList split=line.split (notr (","));
	for (int i=0; i<split.size (); ++i)
		split[i]=split[i].simplified ();

	// Set the attributes from the split parts
	int n=split.count ();
	if (n>=0) launchMethod.id=split[0].toLongLong ();
	if (n>=1)
	{
		if      (split[1].toLower ()==notr ("winch") ) launchMethod.type=typeWinch;
		else if (split[1].toLower ()==notr ("airtow")) launchMethod.type=typeAirtow;
		else if (split[1].toLower ()==notr ("self")  ) launchMethod.type=typeSelf;
		else if (split[1].toLower ()==notr ("other") ) launchMethod.type=typeOther;
		else
		{
			log_error (notr ("Unknown launch method type in LaunchMethod::parseConfigLine (QString)"));
			launchMethod.type=typeOther;
		}
	}
	else
	{
		launchMethod.type=typeOther;
	}
	if (n>=2) launchMethod.towplaneRegistration=split[2];
	if (n>=3) launchMethod.name=split[3];
	if (n>=4) launchMethod.shortName=split[4];
	if (n>=5) launchMethod.keyboardShortcut=split[5];
	if (n>=6) launchMethod.logString=split[6];
	if (n>=7 && split[7].toLower ()==notr ("false")) launchMethod.personRequired=false;

	return launchMethod;
}


// ****************
// ** Formatting **
// ****************

QString LaunchMethod::toString () const
{
	if (personRequired)
		return qnotr ("id=%1, description=%2 (%3), type=%4, person required")
			.arg (id).arg (name).arg (shortName).arg (typeString (type));
	else
		return qnotr ("id=%1, description=%2 (%3), type=%4, person not required")
			.arg (id).arg (name).arg (shortName).arg (typeString (type));
}

QString LaunchMethod::nameWithShortcut () const
{
	if (keyboardShortcut.isEmpty ())
		return qnotr (" ")+name;
	else
		return QString (keyboardShortcut)+qnotr (" - ")+name;
}

QString LaunchMethod::getDisplayName () const
{
	return name;
}


// ******************
// ** Type methods **
// ******************

QString LaunchMethod::typeString (LaunchMethod::Type type)
{
	switch (type)
	{
		case typeWinch : return qApp->translate ("LaunchMethod", "winch launch"); break;
		case typeAirtow: return qApp->translate ("LaunchMethod", "airtow"); break;
		case typeSelf  : return qApp->translate ("LaunchMethod", "self launch"); break;
		case typeOther : return qApp->translate ("LaunchMethod", "other"); break;
	}

	return notr ("???");
}

QList<LaunchMethod::Type> LaunchMethod::listTypes ()
{
	return QList<Type> ()
		<< typeWinch
		<< typeAirtow
		<< typeSelf
		<< typeOther
		;
}


// *****************
// ** ObjectModel **
// *****************

int LaunchMethod::DefaultObjectModel::columnCount () const
{
	return 9;
}

QVariant LaunchMethod::DefaultObjectModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return qApp->translate ("LaunchMethod::DefaultObjectModel", "Name");
		case 1: return qApp->translate ("LaunchMethod::DefaultObjectModel", "Short name");
		case 2: return qApp->translate ("LaunchMethod::DefaultObjectModel", "Logbook label");
		case 3: return qApp->translate ("LaunchMethod::DefaultObjectModel", "Keyboard shortcut");
		case 4: return qApp->translate ("LaunchMethod::DefaultObjectModel", "Type");
		case 5: return qApp->translate ("LaunchMethod::DefaultObjectModel", "Towplane");
		case 6: return qApp->translate ("LaunchMethod::DefaultObjectModel", "Person required");
		case 7: return qApp->translate ("LaunchMethod::DefaultObjectModel", "Comments");
		// TODO remove from DefaultObjectModel?
		case 8: return qApp->translate ("LaunchMethod::DefaultObjectModel", "ID");
	}

	assert (false);
	return QVariant ();
}

QVariant LaunchMethod::DefaultObjectModel::displayData (const LaunchMethod &object, int column) const
{
	switch (column)
	{
		case 0: return object.name;
		case 1: return object.shortName;
		case 2: return object.logString;
		case 3: return object.keyboardShortcut;
		case 4: return firstToUpper (typeString (object.type));
		case 5: return object.isAirtow ()?object.towplaneRegistration:notr ("-");
		case 6: return object.personRequired?qApp->translate ("LaunchMethod", "No"):qApp->translate ("LaunchMethod", "No");
		case 7: return object.comments;
		case 8: return object.id;
	}

	assert (false);
	return QVariant ();
}


// *******************
// ** SQL interface **
// *******************

QString LaunchMethod::dbTableName ()
{
	return notr ("launch_methods");
}

QString LaunchMethod::selectColumnList ()
{
	return notr ("id,name,short_name,log_string,keyboard_shortcut,type,towplane_registration,person_required,comments");
}

LaunchMethod LaunchMethod::createFromResult (const Result &result)
{
	LaunchMethod l (result.value (0).toLongLong ());

	l.name                 = result.value (1).toString ();
	l.shortName            = result.value (2).toString ();
	l.logString            = result.value (3).toString ();
	l.keyboardShortcut     = result.value (4).toString ();
	l.type                 = typeFromDb (
	                         result.value (5).toString ());
	l.towplaneRegistration = result.value (6).toString ();
	l.personRequired       = result.value (7).toBool   ();
	l.comments             = result.value (8).toString ();

	return l;
}

LaunchMethod LaunchMethod::createFromDataMap(const QMap<QString,QString> map)
{
    LaunchMethod l (map["id"].toLongLong());

    l.name = map["name"];
    l.shortName = map["short_name"];
    l.logString = map["log_string"];
    l.keyboardShortcut = map["keyboard_shortcut"];
    l.type = typeFromDb(map["type"]);
    l.towplaneRegistration = map["towplane_registration"];
    l.personRequired = map["person_required"].toInt();
    l.comments = map["comments"];

    return l;
}

QString LaunchMethod::insertColumnList ()
{
	return notr ("name,short_name,log_string,keyboard_shortcut,type,towplane_registration,person_required,comments");
}

QString LaunchMethod::insertPlaceholderList ()
{
	return notr ("?,?,?,?,?,?,?,?");
}

void LaunchMethod::bindValues (Query &q) const
{
	q.bind (name);
	q.bind (shortName);
	q.bind (logString);
	q.bind (keyboardShortcut);
	q.bind (typeToDb (type));
	q.bind (towplaneRegistration);
	q.bind (personRequired);
	q.bind (comments);
}

QList<LaunchMethod> LaunchMethod::createListFromResult (Result &result)
{
	QList<LaunchMethod> list;
	while (result.next ()) list.append (createFromResult (result));
	return list;
}

// *** Enum mappers
QString LaunchMethod::typeToDb (LaunchMethod::Type type)
{
	switch (type)
	{
		case typeWinch  : return notr ("winch");
		case typeAirtow : return notr ("airtow");
		case typeSelf   : return notr ("self");
		case typeOther  : return notr ("other");
		// no default
	}

	assert (false);
	return notr ("?");
}

LaunchMethod::Type LaunchMethod::typeFromDb (QString type)
{
	if      (type==notr ("winch") ) return typeWinch;
	else if (type==notr ("airtow")) return typeAirtow;
	else if (type==notr ("self")  ) return typeSelf;
	else if (type==notr ("other") ) return typeOther;

	// Unknown type
	return typeOther;
}

