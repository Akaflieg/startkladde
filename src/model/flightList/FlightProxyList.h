/*
 * FlightProxyList.h
 *
 *  Created on: Sep 1, 2009
 *      Author: Martin Herrmann
 */

#ifndef FLIGHTPROXYLIST_H_
#define FLIGHTPROXYLIST_H_

#include "src/model/Flight.h"
#include "src/model/objectList/AbstractObjectList.h"
#include "src/db/dbId.h"

class FlightReference;
class LaunchMethod;
class Cache;

class FlightProxyList: public AbstractObjectList<Flight>
{
	Q_OBJECT

	public:
		FlightProxyList (Cache &cache, QObject *parent=NULL);
		virtual ~FlightProxyList ();

		virtual void setSourceModel (AbstractObjectList<Flight> *sourceModel);

		// AbstractObjectList<Flight> methods
		virtual int size () const;
		virtual const Flight &at (int index) const;
		virtual QList<Flight> getList () const;

		virtual int findTowref (int index) const;
		virtual int findModelIndex (const FlightReference &flight) const;
		//virtual Maybe<Flight> findFlight (const FlightReference &flight) const; // Untested
		virtual FlightReference getFlightReference (int modelIndex) const;

	protected:
		virtual bool isAirtow (const Flight &flight, LaunchMethod *launchMethod) const;
		virtual void addTowflightFor (const Flight &flight, const LaunchMethod &launchMethod);
		virtual void updateTowflight (dbId id, int towflightIndex);

		virtual int findFlight (dbId id) const;
		virtual int findTowflight (dbId id) const;

		virtual bool modelIndexIsFlight (int index) const;
		virtual bool modelIndexIsTowflight (int index) const;

		virtual int flightIndexToModelIndex (int flightIndex) const { return flightIndex; }
		virtual int modelIndexToFlightIndex (int modelIndex) const { return modelIndex; }
		virtual int towflightIndexToModelIndex (int towflightIndex) const;
		virtual int modelIndexToTowflightIndex (int modelIndex) const;

	protected slots:
		// TODO: on layoutChanged, reset this model
		virtual void sourceModel_dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);
		virtual void sourceModel_modelAboutToBeReset ();
		virtual void sourceModel_modelReset ();
		virtual void sourceModel_rowsAboutToBeInserted (const QModelIndex &parent, int start, int end);
		virtual void sourceModel_rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end);
		virtual void sourceModel_rowsInserted (const QModelIndex &parent, int start, int end);
		virtual void sourceModel_rowsRemoved (const QModelIndex &parent, int start, int end);

		virtual void sourceModel_destroyed ();

	private:
		void connectSourceModel (AbstractObjectList<Flight> *sourceModel);

		Cache &cache;

		// The model that contains the flights
		AbstractObjectList<Flight> *_sourceModel;

		// The towflights added by this proxy
		QList<Flight> towflights;
};

#endif
