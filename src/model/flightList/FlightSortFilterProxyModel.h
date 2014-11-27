/*
 * FlightSortFilterProxyModel.h
 *
 *  Created on: Sep 3, 2009
 *      Author: Martin Herrmann
 */

#ifndef FLIGHTSORTFILTERPROXYMODEL_H_
#define FLIGHTSORTFILTERPROXYMODEL_H_

#include <QSortFilterProxyModel>
#include <QSet>

#include "src/db/dbId.h"

class Cache;
class FlightReference;

class FlightSortFilterProxyModel: public QSortFilterProxyModel
{
	Q_OBJECT

	public:
		FlightSortFilterProxyModel (Cache &cache, QObject *parent);
		virtual ~FlightSortFilterProxyModel ();

	public slots:
		virtual void setShowPreparedFlights        (bool showPreparedFlights       ) { this->showPreparedFlights       =showPreparedFlights       ; invalidate (); }
		virtual void setHideFinishedFlights        (bool hideFinishedFlights       ) { this->hideFinishedFlights       =hideFinishedFlights       ; invalidate (); }
		virtual void setAlwaysShowExternalFlights  (bool alwaysShowExternalFlights ) { this->alwaysShowExternalFlights =alwaysShowExternalFlights ; invalidate (); }
		virtual void setAlwaysShowErroneousFlights (bool alwaysShowErroneousFlights) { this->alwaysShowErroneousFlights=alwaysShowErroneousFlights; invalidate (); }
		virtual void setCustomSorting (bool customSorting);
		virtual bool getCustomSorting () const { return customSorting; }
		virtual void sortCustom ();
		virtual void setFlarmIdColumn (int column);
		virtual void setIdColumn (int column);

		virtual void setForceVisible (const FlightReference &flight, bool forceVisible);

	protected:
		virtual bool filterAcceptsRow (int sourceRow, const QModelIndex &sourceParent) const;
		virtual bool filterAcceptsColumn (int sourceColumn, const QModelIndex &sourceParent) const;
		virtual bool lessThan (const QModelIndex &left, const QModelIndex &right) const;

	protected slots:
		void settingsChanged ();

	private:
		Cache &cache;

		// Row filter options
		bool showPreparedFlights; // TODO: remove this, the main window takes care of that
		bool hideFinishedFlights;
		bool alwaysShowExternalFlights;
		bool alwaysShowErroneousFlights;
		QSet<FlightReference> flightsForceVisible;

		// Column filter options
		int flarmIdColumn;
		int idColumn;
		bool acceptDebugColumns;

		// Sort options
		bool customSorting;
};

#endif
