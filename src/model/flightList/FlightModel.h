/*
 * FlightModel.h
 *
 *  Created on: Aug 30, 2009
 *      Author: Martin Herrmann
 */

#ifndef FLIGHTMODEL_H_
#define FLIGHTMODEL_H_

#include <QVariant>

#include "src/model/objectList/ObjectModel.h"
#include "src/model/objectList/ColumnInfo.h"

class Flight;
class Cache;

/**
 * Unlike the models for Person, Plane and LaunchMethod, this is not part of the
 * respective class because it is much more complex.
 *
 * For performance reasons, this model caches the translated strings. On
 * language change, the values have to be updated. This would be easy, but
 * updating the strings is not enough, we also have to signal the user
 * (typically an ObjectListModel<Flight>) that the model data has become
 * invalid (like QAbstractItemModel's reset method), but there is no mechanism
 * for that at the moment. Therefore, on language change, someone must call
 * this model's updateTranslations method and make sure that the data is read
 * from the model again (ObjectListModel even exposes the reset method for this
 * purpose).
 * Note that this only applies to the header data and the button texts - data
 * that is read from (and translated in) the flight is retranslated without
 * these measures, as the QAbstractItemModel resets (?).
 * TODO change this, add a modelReset signal (headerDataChanged could also be
 * used, but the buttons have to be updated as well) (and unexpose
 * ObjectListModel's reset method)
 */
class FlightModel: public ObjectModel<Flight>, public ColumnInfo
{
	public:
		FlightModel (Cache &cache);
		virtual ~FlightModel ();

		void setColorEnabled (bool colorEnabled);

		virtual int columnCount () const;
		virtual QVariant displayHeaderData (int column) const;
		virtual QVariant data (const Flight &flight, int column, int role=Qt::DisplayRole) const;

		virtual int pilotColumn        () const { return  3; }
		virtual int departButtonColumn () const { return  6; }
		virtual int landButtonColumn   () const { return  7; }
		virtual int durationColumn     () const { return  8; }
        virtual int vfIdColumn         () const { return 15; }
        virtual int idColumn           () const { return 16; }
        virtual int flarmIdColumn      () const { return 17; }


		// ColumnInfo methods
		virtual QString columnName (int columnIndex) const;
		virtual QString sampleText (int columnIndex) const;

		void updateTranslations ();

	protected:
		virtual QVariant registrationData (const Flight &flight, int role) const;
		virtual QVariant planeTypeData (const Flight &flight, int role) const;
		virtual QVariant pilotData (const Flight &flight, int role) const;
		virtual QVariant copilotData (const Flight &flight, int role) const;
		virtual QVariant launchMethodData (const Flight &flight, int role) const;
		virtual QVariant departureTimeData (const Flight &flight, int role) const;
		virtual QVariant landingTimeData (const Flight &flight, int role) const;
		virtual QVariant durationData (const Flight &flight, int role) const;

	private:
		Cache &cache;
		
		// Cached strings
		QString headerTextRegistration;
		QString headerTextModel;
		QString headerTextType;
		QString headerTextPilot;
		QString headerTextCopilot;
        QString headerTextNumCrew;
        QString headerTextNumPax;
		QString headerTextLaunchMethod;
		QString headerTextDeparture;
		QString headerTextLanding;
		QString headerTextDuration;
		QString headerTextNumLandings;
		QString headerTextDepartureLocation;
		QString headerTextLandingLocation;
		QString headerTextComments;
		QString headerTextAccountingNotes;
		QString headerTextDate;
		QString headerTextId;
		QString headerTextFlarmId;
        QString headerTextVFUploaded;

		bool colorEnabled;

		bool flarmIdVisible;
};

#endif
