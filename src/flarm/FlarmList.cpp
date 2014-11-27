#include "src/flarm/FlarmList.h"

#include <cassert>
#include <iostream>

#include "src/nmea/NmeaDecoder.h"
#include "src/nmea/PflaaSentence.h"
#include "src/numeric/Velocity.h"
#include "src/util/qString.h"
#include "src/flarm/algorithms/PlaneLookup.h"
#include "src/flarm/flarmNet/FlarmNetRecord.h"

// FIXME: the data structures (including FlarmRecord and this list) stink, we
// should have a proper model/view/controller architecture

// TODO: we have to take care not to let the list and the map get inconsistent.
// We might want to encapsulate that, either in a base class or by placing the
// modification stuff in a controller class.
// A more comprehensive solution would be a separate cache class which listens
// to the signals from the object list.

// TODO: there should be only one signal called flarmEvent with a FlarmEvent
// parameter containing the Flarm ID, the type, and potentially the system time
// and/or GPS time of the event.

FlarmList::FlarmList (QObject *parent) :
	AbstractObjectList<FlarmRecord> (parent),
	nmeaDecoder (NULL), dbManager (NULL)
{
}

FlarmList::~FlarmList ()
{
}


// ******************
// ** NMEA decoder **
// ******************

void FlarmList::setNmeaDecoder (NmeaDecoder *nmeaDecoder)
{
	if (nmeaDecoder==this->nmeaDecoder)
		return;

	if (this->nmeaDecoder)
	{
		this->nmeaDecoder->disconnect (this);
	}

	this->nmeaDecoder=nmeaDecoder;

	if (this->nmeaDecoder)
	{
		connect (this->nmeaDecoder, SIGNAL (pflaaSentence (const PflaaSentence &)), this, SLOT (pflaaSentence (const PflaaSentence &)));
	}
}

void FlarmList::setDatabase (DbManager *dbManager)
{
	this->dbManager = dbManager;
}


// *********************
// ** Data processing **
// *********************

void FlarmList::pflaaSentence (const PflaaSentence &sentence)
{
	if (!sentence.isValid ()) return;

	// Find the Flarm record for that plane (i. e., Flarm ID)
	QModelIndex recordIndex = byFlarmId.value (sentence.flarmId);

	if (recordIndex.isValid ())
	{
		// A Flarm record was found
		flarmRecords[recordIndex.row ()]->processPflaaSentence (sentence);
		QAbstractItemModel::dataChanged (recordIndex, recordIndex);
	}
	else
	{
		// There is no Flarm record for that plane yet. Create one, process
		// the sentence and add it to the list.
		FlarmRecord *record = new FlarmRecord (this, sentence.flarmId);

		connect (record, SIGNAL (departureDetected  (const QString &)), this, SIGNAL (departureDetected  (const QString &)));
		connect (record, SIGNAL (landingDetected    (const QString &)), this, SIGNAL (landingDetected    (const QString &)));
		connect (record, SIGNAL (touchAndGoDetected (const QString &)), this, SIGNAL (touchAndGoDetected (const QString &)));
		connect (record, SIGNAL (remove (const QString &)), this, SLOT (removeFlarmRecord (const QString &)));

		record->processPflaaSentence (sentence);

		// Let's see if we can find out anything about this plane
		PlaneLookup::Result result=PlaneLookup (dbManager->getCache ()).lookupPlane (sentence.flarmId);
		if (result.plane.isValid ())
		{
			record->setRegistration (result.plane.getValue ().registration);
			record->setCategory (result.plane.getValue ().category);

			if (result.flarmNetRecord.isValid ())
				record->setFrequency (result.flarmNetRecord.getValue ().frequency);
		}
		else if (result.flarmNetRecord.isValid ())
		{
			record->setRegistration (result.flarmNetRecord.getValue ().registration);
			record->setFrequency (result.flarmNetRecord.getValue ().frequency);
		}

		// FIXME separate method for adding/removing, updating the cache
		// Add it to the list and hash
		int newRow = flarmRecords.size ();

		QAbstractItemModel::beginInsertRows (QModelIndex (), newRow, newRow);

		// Add to list
		flarmRecords.append (record);

		// Add to cache
		Q_ASSERT (!byFlarmId.contains (sentence.flarmId));
		byFlarmId.insert (sentence.flarmId, QPersistentModelIndex (index (newRow)));

		QAbstractItemModel::endInsertRows ();

		// FIXME when the Flarm record changes, emit a signal, or only do changes
		// from here
	}
}

// **********************
// ** Controller slots **
// **********************

// It's not a proper controller in the MVC sense, it's integrated with the
// model. We may want to change this some time.

void FlarmList::removeFlarmRecord (const QString &flarmId)
{
	QModelIndex index = byFlarmId.value (flarmId);
	if (index.isValid ())
	{
		int row=index.row ();

		QAbstractItemModel::beginRemoveRows (QModelIndex (), row, row);

		// Remove from list
		flarmRecords.removeAt (row);

		// Remove from cache
		Q_ASSERT (byFlarmId.contains (flarmId));
		byFlarmId.remove (flarmId);

		QAbstractItemModel::endRemoveRows ();
	}
}

// ********************************
// ** AbstractObjectList methods **
// ********************************

int FlarmList::size () const
{
	return flarmRecords.size ();
}

const FlarmRecord &FlarmList::at (int index) const
{
	return *flarmRecords.at (index);
}

QList<FlarmRecord> FlarmList::getList () const
{
	// We cannot implement this because we cannot store FlarmRecord in a
	// Qt container. This list stores pointer to FlarmRecord.
	assert (!"Not supported");
	exit (1);
}
