#include "FlightProxyList.h"

#include <cassert>

#include "src/model/Flight.h"
#include "src/db/cache/Cache.h"
#include "src/model/LaunchMethod.h"
#include "src/FlightReference.h"

FlightProxyList::FlightProxyList (Cache &cache, QObject *parent):
	AbstractObjectList<Flight> (parent),
	cache (cache),
	_sourceModel (NULL)
{
	// TODO: if there is no self launch method, the flight will be red
	// in the table, but not show an error in the editor (no longer true?)

}

FlightProxyList::~FlightProxyList ()
{
}


// ***********
// ** Model **
// ***********

void FlightProxyList::connectSourceModel (AbstractObjectList<Flight> *sourceModel)
{
	// Some signals from the source model can be re-emitted without change
#define reemitSignal(signal) do { QObject::connect (sourceModel, SIGNAL (signal), this, SIGNAL (signal)); } while (0)
	reemitSignal (columnsAboutToBeInserted (const QModelIndex &, int, int));
	reemitSignal (columnsAboutToBeRemoved  (const QModelIndex &, int, int));
	reemitSignal (columnsInserted          (const QModelIndex &, int, int));
	reemitSignal (columnsRemoved           (const QModelIndex &, int, int));
	reemitSignal (headerDataChanged (Qt::Orientation, int, int));
	reemitSignal (layoutAboutToBeChanged ());
	reemitSignal (layoutChanged ());
#undef reemitSignal

#define receiveSignal(signal) do {QObject::connect (sourceModel, SIGNAL (signal), this, SLOT (sourceModel_ ## signal)); } while (0)
	receiveSignal (dataChanged (const QModelIndex &, const QModelIndex &));
	receiveSignal (modelAboutToBeReset ());
	receiveSignal (modelReset ());
	receiveSignal (rowsAboutToBeInserted (const QModelIndex &, int, int));
	receiveSignal (rowsAboutToBeRemoved  (const QModelIndex &, int, int));
	receiveSignal (rowsInserted          (const QModelIndex &, int, int));
	receiveSignal (rowsRemoved           (const QModelIndex &, int, int));
#undef receiveSignal
}

void FlightProxyList::setSourceModel (AbstractObjectList<Flight> *sourceModel)
{
	if (_sourceModel==sourceModel)
		return;

	// Disconnect the old source model's signals
	if (_sourceModel)
		_sourceModel->disconnect (this);

	// Store the new source model
	_sourceModel=sourceModel;

	// Connect the new source model's signals
	connectSourceModel (sourceModel);

	// Clear internal data and notify views
	sourceModel_modelAboutToBeReset ();
	sourceModel_modelReset ();
}

void FlightProxyList::sourceModel_destroyed ()
{
	_sourceModel=NULL;
}


// **************************
// ** Towflight management **
// **************************

// TODO should be in Flight
// TODO This also adds towflights for prepared flights with an airtow launch
// method. Do we want that? Note that they are filtered out in the
// FlightSortFilterProxyModel, but we still find a towref for them. This stinks.
// One thing to consider is that we'll want to split the flight and the
// towflight in the database some time. In that case, we'll have to create them
// together (so we can store the pilot's name), so there are actually prepared
// towflights.
bool FlightProxyList::isAirtow (const Flight &flight, LaunchMethod *launchMethod) const
{
	// No launch method => no airtow
	if (!idValid (flight.getLaunchMethodId ()))
		return false;

	try
	{
		LaunchMethod lm=cache.getObject<LaunchMethod> (flight.getLaunchMethodId ());
		if (launchMethod) *launchMethod=lm;
		return lm.isAirtow ();
	}
	catch (Cache::NotFoundException &)
	{
		// Launch method not found => no airtow
		return false;
	}

	return false;
}

void FlightProxyList::addTowflightFor (const Flight &flight, const LaunchMethod &launchMethod)
{
	// Determine the ID of the towplane
	// TODO code duplication with updateTowflight
	dbId towplaneId=invalidId;

	if (launchMethod.towplaneKnown ())
		towplaneId=cache.getPlaneIdByRegistration (launchMethod.towplaneRegistration);
	else
		towplaneId=flight.getTowplaneId ();

	// For the launch method, use an invalid ID. The FlightModel recognizes
	// towflights by their type and displays the launch method as self launch
	towflights.append (flight.makeTowflight (towplaneId, invalidId));
}

void FlightProxyList::updateTowflight (dbId id, int towflightIndex)
{
	try
	{
		Flight flight=cache.getObject<Flight> (id);
		LaunchMethod launchMethod;
		if (isAirtow (flight, &launchMethod)) // TODO log error if not
		{
			// Determine the ID of the towplane
			// TODO code duplication with addTowflightFor
			dbId towplaneId=invalidId;

			if (launchMethod.towplaneKnown ())
				towplaneId=cache.getPlaneIdByRegistration (launchMethod.towplaneRegistration);
			else
				towplaneId=flight.getTowplaneId ();

			dbId selfLaunchId=cache.getLaunchMethodByType (LaunchMethod::typeSelf);

			towflights.replace (towflightIndex, flight.makeTowflight (towplaneId, selfLaunchId));
		}
	}
	catch (Cache::NotFoundException &)
	{
		// Do nothing // TODO log error
	}
}

/**
 *
 * @param id
 * @return a flight index
 */
int FlightProxyList::findFlight (dbId id) const
{
	if (!_sourceModel)
		return -1;

	for (int i=0; i<_sourceModel->size (); ++i)
		if (_sourceModel->at (i).getId ()==id)
			return i;

	return -1;
}

/**
 *
 * @param id
 * @return a towflight index
 */
int FlightProxyList::findTowflight (dbId id) const
{
	for (int i=0; i<towflights.size (); ++i)
		if (towflights[i].getId ()==id)
			return i;

	return -1;
}

bool FlightProxyList::modelIndexIsFlight (int index) const
{
	if (!_sourceModel)
		return false;

	return index<_sourceModel->size ();
}

bool FlightProxyList::modelIndexIsTowflight (int index) const
{
	int towflightIndex=modelIndexToTowflightIndex (index);
	return (towflightIndex>=0 && towflightIndex<towflights.size ());
}

int FlightProxyList::towflightIndexToModelIndex (int towflightIndex) const
{
	if (!_sourceModel)
		return -1;

	if (towflightIndex<0)
		return -1;

	return towflightIndex+_sourceModel->size ();
}

int FlightProxyList::modelIndexToTowflightIndex (int modelIndex) const
{
	if (!_sourceModel)
		return -1;

	if (modelIndex<0)
		return -1;

	return modelIndex-_sourceModel->size ();
}

/**
 *
 * @param index
 * @return a model index
 */
int FlightProxyList::findTowref (int index) const
{
	dbId id=at (index).getId ();

	if (modelIndexIsFlight (index))
	{
		int towref=findTowflight (id);
		if (towref>=0)
			return towflightIndexToModelIndex (towref);
		else
			return -1;
	}
	else if (modelIndexIsTowflight (index))
	{
		int towref=findFlight (id);
		if (towref>=0)
			return flightIndexToModelIndex (towref);
		else
			return -1;
	}
	else
	{
		return -1;
	}
}

int FlightProxyList::findModelIndex (const FlightReference &flight) const
{
	if (!flight.isValid ())
		return -1;

	if (flight.towflight ())
	{
		int towflightIndex=findTowflight (flight.id ());
		return towflightIndexToModelIndex (towflightIndex); // Handles <0
	}
	else
	{
		int flightIndex=findFlight (flight.id ());
		return flightIndexToModelIndex (flightIndex); // Handles <0
	}
}

//Maybe<Flight> FlightProxyList::findFlight (const FlightReference &flight) const
//{
//	int modelIndex=findModelIndex (flight);
//	return at (modelIndex);
//}

FlightReference FlightProxyList::getFlightReference (int modelIndex) const
{
	if (modelIndex<0)
		return FlightReference::invalid;

	return FlightReference (at (modelIndex));
}




// ****************************************
// ** AbstractObjectList<Flight> methods **
// ****************************************

int FlightProxyList::size () const
{
	if (!_sourceModel)
		return 0;

	return _sourceModel->size ()+towflights.size ();
}

// TODO return Maybe<Flight>, and check for _sourceModel!=NULL
const Flight &FlightProxyList::at (int index) const
{
	assert (_sourceModel);

	int numFlights=_sourceModel->size ();

	if (index<numFlights)
		return _sourceModel->at (index);
	else
		return towflights.at (modelIndexToTowflightIndex (index));
}

QList<Flight> FlightProxyList::getList () const
{
	if (!_sourceModel)
		return QList<Flight> ();

	return _sourceModel->getList () + towflights;
}



// ************************
// ** Source model slots **
// ************************

// TODO: when emitting/reemitting signals, should we generate a new parent index?

void FlightProxyList::sourceModel_dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	if (!_sourceModel)
		return;

	// Emit the change signal for the flight
	emit dataChanged (topLeft, bottomRight);

	// For each of the flights, if there is a corresponding towflight, emit
	// a change signal for the towflight
	for (int i=topLeft.row (); i<=bottomRight.row (); ++i)
	{
		const Flight &flight=_sourceModel->at (i);
		dbId id=flight.getId ();

		// Determine the launch method and whether the flight is an airtow
		LaunchMethod launchMethod;
		bool flightIsAirtow=isAirtow (_sourceModel->at (i), &launchMethod);

		// We may or may not have a towflight with that ID, regardless of
		// whether the flight is an airtow, because that may have changed.
		int towflightIndex=findTowflight (id);

		// TODO code duplication with removing/adding
		if (flightIsAirtow)
		{
			// Airtow - make sure that there is a towflight

			if (towflightIndex>=0)
			{
				// Towflight already present
				updateTowflight (id, towflightIndex);

				int modelIndex=towflightIndexToModelIndex (towflightIndex);
				// TODO is this the correct parent to use?
				QModelIndex parent=topLeft.parent ();
				emit dataChanged (createIndex (modelIndex, 0), createIndex (modelIndex, columnCount (parent)-1));
			}
			else
			{
				// No towflight yet - add it

				// TODO code duplication with rowsInserted - should be in addTowflightFor
				int towflightIndex=towflights.size ();
				int modelIndex=towflightIndexToModelIndex (towflightIndex);

				beginInsertRows (QModelIndex (), modelIndex, modelIndex);
				addTowflightFor (flight, launchMethod);
				endInsertRows ();
			}

		}
		else
		{
			// No airtow - make sure that there is no towflight
			if (towflightIndex>=0)
			{
				// We have a towflight for this flight - remove it.
				int modelIndex=towflightIndexToModelIndex (towflightIndex);

				beginRemoveRows (QModelIndex (), modelIndex, modelIndex);
				towflights.removeAt (towflightIndex);
				endRemoveRows ();
			}
		}
	}
}

void FlightProxyList::sourceModel_modelAboutToBeReset ()
{
	beginResetModel ();
	towflights.clear ();
}

void FlightProxyList::sourceModel_modelReset ()
{
	if (_sourceModel)
	{
		LaunchMethod launchMethod;

		const QList<Flight> flights=_sourceModel->getList ();
		foreach (const Flight &flight, flights)	// For each flight in the source model
			if (isAirtow (flight, &launchMethod))	// Is it an airtow?
				addTowflightFor (flight, launchMethod);	// Add the towflight to the towflight list (don't notify listeners, we'll reset later)
	}

	endResetModel ();
}

void FlightProxyList::sourceModel_rowsAboutToBeInserted (const QModelIndex &parent, int start, int end)
{
	beginInsertRows (parent, start, end);
}

void FlightProxyList::sourceModel_rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end)
{
	if (!_sourceModel)
		return;

	// Flights will be removed. Remove the corresponding towflights, if there
	// are any.
	// We have to remove the towflights in rowsAboutToBeRemoved, because in
	// rowsRemoved the flights are already gone. We may not call endRemoveRows
	// for the flights here, because they are still there.

	// Iterate over the flights. For each flight, if there is a corresponding
	// towflight, remove the towflight.
	for (int i=start; i<=end; ++i)
	{
		// The towflight has the same ID as the flight
		dbId id=_sourceModel->at (i).getId ();

		int towflightIndex=findTowflight (id);
		if (towflightIndex>=0)
		{
			// We have a towflight for this flight - remove it.
			int modelIndex=towflightIndexToModelIndex (towflightIndex);

			beginRemoveRows (parent, modelIndex, modelIndex);
			towflights.removeAt (towflightIndex);
			endRemoveRows ();
		}
	}

	// Begin the removing of the flights after the towflights have been
	// removed, to avoid overlapping removes.
	beginRemoveRows (parent, start, end);
}

void FlightProxyList::sourceModel_rowsInserted (const QModelIndex &parent, int start, int end)
{
	if (!_sourceModel)
		return;

	endInsertRows ();

	// Iterate over the inserted flights, adding the corresponding towflights.
	for (int i=start; i<=end; ++i)
	{
		const Flight &flight=_sourceModel->at (i);

		LaunchMethod launchMethod;
		if (isAirtow (flight, &launchMethod))
		{
			// TODO code duplication with sourceModel_dataChanged - this should
			// be in addTowflightFor
			int towflightIndex=towflights.size ();
			int modelIndex=towflightIndexToModelIndex (towflightIndex);

			beginInsertRows (parent, modelIndex, modelIndex);
			addTowflightFor (flight, launchMethod);
			endInsertRows ();
		}
	}
}

void FlightProxyList::sourceModel_rowsRemoved (const QModelIndex &parent, int start, int end)
{
	// We don't have to do anything to the towflight list - corresponding
	// towflights were removed in rowsAboutToBeRemoved.
	(void)parent;
	(void)start;
	(void)end;

	endRemoveRows ();
}
