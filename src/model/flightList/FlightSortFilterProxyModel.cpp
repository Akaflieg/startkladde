#include "FlightSortFilterProxyModel.h"

#include <cassert>

#include "src/FlightReference.h"
#include "src/config/Settings.h"
#include "src/model/Flight.h"
#include "src/model/objectList/ObjectListModel.h"

FlightSortFilterProxyModel::FlightSortFilterProxyModel (Cache &cache, QObject *parent):
	QSortFilterProxyModel (parent),
	cache (cache),
	showPreparedFlights (true),
	hideFinishedFlights (false), alwaysShowExternalFlights (true), alwaysShowErroneousFlights (true),
    flarmIdColumn (-1), idColumn (-1), vfIdColumn(-1),
	customSorting (true)
{
    Settings& s = Settings::instance ();
    acceptDebugColumns= s.enableDebug;
    acceptVfColumns = s.vfUploadEnabled;

	connect (&Settings::instance (), SIGNAL (changed ()), this, SLOT (settingsChanged ()));
}

FlightSortFilterProxyModel::~FlightSortFilterProxyModel ()
{
}

void FlightSortFilterProxyModel::setForceVisible (const FlightReference &flight, bool forceVisible)
{
	if (forceVisible)
		flightsForceVisible.insert (flight);
	else
		flightsForceVisible.remove (flight);

	invalidate ();
}

bool FlightSortFilterProxyModel::filterAcceptsRow (int sourceRow, const QModelIndex &sourceParent) const
{
	(void)sourceParent;

	// If the model is not an ObjectListModel<Flight>, the filter is not active
	ObjectListModel<Flight> *flightList=dynamic_cast<ObjectListModel<Flight> *> (sourceModel ());
	if (!flightList) return true;

	// Get the flight from the model
	const Flight &flight=flightList->at (sourceRow);

	if (flightsForceVisible.contains (FlightReference (flight)))
		return true;

	if (flight.isPrepared ())
	{
		// Prepared flights are hidden if one of these is true:
		//   - showPreparedFlights is false
		//   - the flight is a towflight

		if (!showPreparedFlights) return false;
		if (flight.isTowflight ()) return false;

		return true;
	}
	else
	{
		if (flight.finished ())
		{
			// Finished flights are shown if one of these is true:
			//   - hideFinishedFlights is false
			//   - the flight is external and alwaysShowExternalFlights is true
			//   - the flight is erroneous and alwaysShowExternalFlights is true

			if (!hideFinishedFlights) return true;
			if (alwaysShowExternalFlights && flight.isExternal ()) return true;
			if (alwaysShowErroneousFlights && flight.isErroneous (cache)) return true;

			return false;
		}
		else
		{
			// Flying flights are always shown
			return true;
		}
	}

	// This should not be reached - every leaf in the decision tree above
	// should return.
	assert (false);
	return true;
}

bool FlightSortFilterProxyModel::filterAcceptsColumn (int sourceColumn, const QModelIndex &sourceParent) const
{
	(void)sourceParent;

    if (sourceColumn==flarmIdColumn || sourceColumn==idColumn)
		return acceptDebugColumns;
    else if (sourceColumn ==vfIdColumn)
        return acceptVfColumns;
	else
		return true;
}

bool FlightSortFilterProxyModel::lessThan (const QModelIndex &left, const QModelIndex &right) const
{
	// TODO: when sorting by time etc.: always sort prepared flights last

	ObjectListModel<Flight> *flightList=dynamic_cast<ObjectListModel<Flight> *> (sourceModel ());

	// If the model is not an ObjectListModel<Flight>, use the default sorting.
	// The indicies refer to the source model, so we don't remap them.
	if (customSorting && flightList)
		return flightList->at (left) < flightList->at (right);
	else
		return QSortFilterProxyModel::lessThan (left, right);
}

/**
 * Enables custom sorting
 *
 * Note that this only sets the custom sorting flag that will be evaluated by
 * lessThan. The model will only be sorted when calling #sort (and the column
 * will be ignored if custom sorting is enabled).
 *
 * @param customSorting
 */
void FlightSortFilterProxyModel::setCustomSorting (bool customSorting)
{
	this->customSorting=customSorting;

	// Invalidate the current sorted order so the model will actually be
	// sorted on the next call to sort, even if it's the same column as before.
	this->invalidate ();
}

void FlightSortFilterProxyModel::sortCustom ()
{
	setCustomSorting (true);

	// The sort column will be ignored for custom sorting
	sort (0, Qt::AscendingOrder);
}

void FlightSortFilterProxyModel::setFlarmIdColumn (int column)
{
	flarmIdColumn=column;
}

void FlightSortFilterProxyModel::setIdColumn (int column)
{
	idColumn=column;
}

void FlightSortFilterProxyModel::setVfIdColumn (int column)
{
    vfIdColumn=column;
}

void FlightSortFilterProxyModel::settingsChanged ()
{
	beginResetModel ();
    Settings& s = Settings::instance ();
    acceptDebugColumns= s.enableDebug;
    acceptVfColumns = s.vfUploadEnabled;
	endResetModel ();
}
