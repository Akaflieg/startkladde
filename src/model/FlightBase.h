#ifndef FLIGHTBASE_H_
#define FLIGHTBASE_H_

#include <QString>
#include <QDateTime>

#include "src/db/dbId.h"

#define flight_value_accessor(type, capitalName, name) \
	type get ## capitalName () const { return name; } \
	void set ## capitalName (const type &name) { this->name=name; dataChanged (); } \
	type &refTo ## capitalName () { return name; }

// TODO: inherit from Entity
/**
 * A base class for Flight, encapsulating the properties
 *
 * The purpose of this class is to encapsulate the properties of a flight, even
 * from Flight's methods itself, and allow access to them only via accessors,
 * in order to ensure that cached values are properly invalidated/updated on
 * every property change.
 */
class FlightBase
{
	public:
		// *** Types
		enum Type {
			typeNone,
			typeNormal,
            typeTraining2, typeTraining1, typeAuffrischung,
			typeGuestPrivate, typeGuestExternal,
			typeTow
		};

		enum Mode { modeLocal, modeComing, modeLeaving };

		// *** Construction
		FlightBase ();
		FlightBase (dbId id); // TODO protected (friend Database)?
		virtual ~FlightBase ();

		// *** Attribute accessors
		virtual dbId getId () const { return id; }
		virtual void setId (dbId id) { this->id=id; } // TODO can we do without this?

		flight_value_accessor (dbId, VfId, vfId);
		flight_value_accessor (bool, Uploaded, uploaded);
		flight_value_accessor (dbId, PlaneId, planeId);
		flight_value_accessor (dbId, PilotId, pilotId);
		flight_value_accessor (dbId, CopilotId, copilotId);
        flight_value_accessor (dbId, SupervisorId, supervisorId);
		flight_value_accessor (int, NumCrew, numCrew)
		flight_value_accessor (int, NumPax, numPax)
		flight_value_accessor (Type, Type, type);
		flight_value_accessor (Mode, Mode, mode);

		flight_value_accessor (bool, Departed, departed);
		flight_value_accessor (bool, Landed, landed);
		flight_value_accessor (bool, TowflightLanded, towflightLanded);
		flight_value_accessor (dbId, LaunchMethodId, launchMethodId);
		flight_value_accessor (QString, DepartureLocation, departureLocation);
		flight_value_accessor (QString, LandingLocation, landingLocation);

		flight_value_accessor (QDateTime, DepartureTime, departureTime);
		flight_value_accessor (QDateTime, LandingTime, landingTime);
		flight_value_accessor (int, NumLandings, numLandings);

		flight_value_accessor (dbId, TowplaneId, towplaneId);
		flight_value_accessor (Mode, TowflightMode, towflightMode);
		flight_value_accessor (QString, TowflightLandingLocation, towflightLandingLocation);
		flight_value_accessor (QDateTime, TowflightLandingTime, towflightLandingTime);
		flight_value_accessor (dbId, TowpilotId, towpilotId);

		flight_value_accessor (QString, PilotLastName, pilotLastName);
		flight_value_accessor (QString, PilotFirstName, pilotFirstName);
		flight_value_accessor (QString, CopilotLastName, copilotLastName);
		flight_value_accessor (QString, CopilotFirstName, copilotFirstName);
		flight_value_accessor (QString, TowpilotLastName, towpilotLastName);
		flight_value_accessor (QString, TowpilotFirstName, towpilotFirstName);

		flight_value_accessor (QString, Comments, comments);
		flight_value_accessor (QString, AccountingNotes, accountingNotes);
		/* Only set when the flight is created automatically from a Flarm event */
		flight_value_accessor (QString, FlarmId, flarmId);

	private:
		void initialize (dbId id);
		virtual void dataChanged () const=0;

		// *** Data
		dbId id;

        dbId planeId, pilotId, copilotId, supervisorId;
        int numCrew, numPax;
		Type type;
		Mode mode;

		bool departed, landed, towflightLanded;
		dbId launchMethodId;
		QString departureLocation;
		QString landingLocation;

		QDateTime departureTime;
		QDateTime landingTime;
		int numLandings;

		dbId towplaneId;
		Mode towflightMode;
		QString towflightLandingLocation;
		QDateTime towflightLandingTime;
		dbId towpilotId;

		// Incomplete names
		QString pilotLastName   , pilotFirstName   ;
		QString copilotLastName , copilotFirstName ;
		QString towpilotLastName, towpilotFirstName;

		QString comments;
		QString accountingNotes;
		QString flarmId;
		dbId vfId;
		bool uploaded;
};

#undef flight_value_accessor

#endif
