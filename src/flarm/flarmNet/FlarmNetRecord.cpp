#include "FlarmNetRecord.h"

#include <cassert>

#include <QApplication>
#include <QtCore/QDebug>

#include "src/text.h"
#include "src/db/result/Result.h"
#include "src/db/Query.h"
#include "src/i18n/notr.h"

// ******************
// ** Construction **
// ******************

FlarmNetRecord::FlarmNetRecord ():
        Entity ()
{
	initialize ();
}

FlarmNetRecord::FlarmNetRecord (dbId id):
        Entity (id)
{
	initialize ();
}

void FlarmNetRecord::initialize ()
{
}


// ****************
// ** Formatting **
// ****************

QString FlarmNetRecord::toString () const
{
	return qnotr ("id=%1, flarm_id=%2, registration=%3, callsign=%4, owner=%5, airfield=%6, type=%7, frequency=%8")
		.arg (id)
		.arg (flarmId)
		.arg (registration)
		.arg (callsign)
		.arg (owner)
		.arg (airfield)
		.arg (type)
		.arg (frequency)
		;
}

QString FlarmNetRecord::getDisplayName () const
{
	return flarmId;
}

/**
 * Returns the registration in form "D-XXXX (YY)" if callsign and registration
 * are non-blank, or just one of the components if the other is blank.
 *
 * @see Plane::fullRegistration
 */
QString FlarmNetRecord::fullRegistration (const QString &fullTemplate) const
{
	if (isBlank (callsign))
		return registration;
	else if (isBlank (registration))
		return callsign;
	else
		return fullTemplate.arg (registration, callsign);
}



// *******************
// ** SQL interface **
// *******************

QString FlarmNetRecord::dbTableName ()
{
	return notr ("flarmnet");
}

QString FlarmNetRecord::selectColumnList ()
{
	return notr ("id,flarm_id,registration,callsign,owner,airfield,type,frequency");
}


FlarmNetRecord FlarmNetRecord::createFromResult (const Result &result)
{
	FlarmNetRecord p (result.value (0).toLongLong ());

	p.flarmId      = result.value (1).toString ();
	p.registration = result.value (2).toString ();
	p.callsign     = result.value (3).toString ();
	p.owner        = result.value (4).toString ();
	p.airfield     = result.value (5).toString ();
	p.type         = result.value (6).toString ();
	p.frequency    = result.value (7).toString ();
	// qDebug () << "FlarmNetRecord::createFromResult: " << p.toString() << endl;

	return p;
}

QString FlarmNetRecord::insertColumnList ()
{
	return notr ("flarm_id,registration,callsign,owner,airfield,type,frequency");
}

QString FlarmNetRecord::insertPlaceholderList ()
{
	return notr ("?,?,?,?,?,?,?");
}

void FlarmNetRecord::bindValues (Query &q) const
{
	q.bind (flarmId);
	q.bind (registration);
	q.bind (callsign);
	q.bind (owner);
	q.bind (airfield);
	q.bind (type);
	q.bind (frequency);
}

QList<FlarmNetRecord> FlarmNetRecord::createListFromResult (Result &result)
{
	QList<FlarmNetRecord> list;

	while (result.next ())
		list.append (createFromResult (result));

	return list;
}


// ****************
// ** Conversion **
// ****************

Plane FlarmNetRecord::toPlane (bool guessCategory) const
{
	Plane plane;

	plane.registration = registration;
	plane.club         = owner;
	plane.type         = type;
	plane.callsign     = callsign;
	plane.flarmId      = flarmId;

	if (guessCategory)
		plane.category = Plane::categoryFromRegistration (registration);

	return plane;
}
