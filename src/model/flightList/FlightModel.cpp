#include "FlightModel.h"

#include <cassert>
#include <iostream>

#include <QApplication>
#include <QBrush>

#include "src/itemDataRoles.h"
#include "src/model/Flight.h"
#include "src/model/LaunchMethod.h"
#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/db/cache/Cache.h"
#include "src/util/qString.h"
#include "src/util/qDate.h"
#include "src/i18n/notr.h"
#include "src/config/Settings.h"

FlightModel::FlightModel (Cache &cache):
	cache (cache),
    colorEnabled (true),
    buttonsEnabled (true)
{
	updateTranslations ();
}

FlightModel::~FlightModel ()
{
}

void FlightModel::setColorEnabled (bool colorEnabled)
{
	this->colorEnabled=colorEnabled;
}

void FlightModel::setButtonsEnabled (bool buttonsEnabled)
{
    this->buttonsEnabled = buttonsEnabled;
}

int FlightModel::columnCount () const
{
    return 18;
}

void FlightModel::updateTranslations ()
{
	// The displayHeaderData is queried very often, so the translations are
	// cached. It is unclear whether this actually has a significant
	// performance impact.
	headerTextRegistration      =qApp->translate ("FlightModel", "Reg.");
	headerTextModel             =qApp->translate ("FlightModel", "Model");
	headerTextType              =qApp->translate ("FlightModel", "Type");
	headerTextPilot             =qApp->translate ("FlightModel", "Pilot/Student");
	headerTextCopilot           =qApp->translate ("FlightModel", "Copilot/FI");
    headerTextNumCrew           =qApp->translate ("FlightModel", "Crew members");
    headerTextNumPax            =qApp->translate ("FlightModel", "Passengers");
    headerTextLaunchMethod      =qApp->translate ("FlightModel", "Launch method");
	headerTextDeparture         =qApp->translate ("FlightModel", "Departure");
	headerTextLanding           =qApp->translate ("FlightModel", "Landing");
	headerTextDuration          =qApp->translate ("FlightModel", "Duration");
	headerTextNumLandings       =qApp->translate ("FlightModel", "Ldgs.");
	headerTextDepartureLocation =qApp->translate ("FlightModel", "Departure location");
	headerTextLandingLocation   =qApp->translate ("FlightModel", "Landing location");
    headerTextComments          =qApp->translate ("FlightModel", "Comments");
	headerTextAccountingNotes   =qApp->translate ("FlightModel", "Accounting notes");
	headerTextDate              =qApp->translate ("FlightModel", "Date");
	headerTextId                =qApp->translate ("FlightModel", "ID");
	headerTextFlarmId           =qApp->translate ("FlightModel", "Flarm ID");
    headerTextVFUploaded        =qApp->translate ("FlightModel", "Uploaded to VF");
}

QVariant FlightModel::headerData (int column, int role) const
{
    Settings& s = Settings::instance();

    if (role == Qt::DisplayRole) {

        switch (column)
        {
            case 0: return headerTextRegistration;
            case 1: return headerTextModel;
            case 2: return headerTextType;
            case 3: return s.anonymousMode ? headerTextNumCrew : headerTextPilot;
            case 4: return s.anonymousMode ? headerTextNumPax : headerTextCopilot;
            case 5: return headerTextLaunchMethod;
            case 6: return headerTextDeparture;
            case 7: return headerTextLanding;
            case 8: return headerTextDuration;
            case 9: return headerTextNumLandings;
            case 10: return headerTextDepartureLocation;
            case 11: return headerTextLandingLocation;
            case 12: return headerTextComments;
            case 13: return headerTextAccountingNotes;
            case 14: return headerTextDate;
            case 15: return headerTextVFUploaded;
            case 16: return headerTextId;
            case 17: return headerTextFlarmId;
        }

    }

	// Apparently, an unhandled column can happen when the last flight is deleted
	return QVariant ();
}


QString FlightModel::columnName (int columnIndex) const
{
    Settings& s = Settings::instance();

	switch (columnIndex)
	{
		case 0: return notr ("registration");
		case 1: return notr ("aircraftType");
		case 2: return notr ("flightType");
        case 3: return s.anonymousMode ? notr("numCrew") : notr ("pilot");
        case 4: return s.anonymousMode ? notr("numPax"): notr ("copilot");
		case 5: return notr ("launchMethod");
		case 6: return notr ("departureTime");
		case 7: return notr ("landingTime");
		case 8: return notr ("flightDuration");
		case 9: return notr ("numLandings");
		case 10: return notr ("departureLocation");
		case 11: return notr ("landingLocation");
		case 12: return notr ("comments");
		case 13: return notr ("accountingNote");
		case 14: return notr ("date");
        case 15: return notr ("vfuploaded");
        case 16: return notr ("id");
        case 17: return notr ("flarmId");
	}

	assert (!notr ("Unhandled column"));
	return QString ();
}

QString FlightModel::sampleText (int columnIndex) const
{
	switch (columnIndex)
	{
		case 0: return qApp->translate ("FlightModel", "N99999 (WW)");
		case 1: return qApp->translate ("FlightModel", "DR-400/180");
		case 2: return qApp->translate ("FlightModel", "Passenger (E)");
		case 3: return qApp->translate ("FlightModel", "Xxxxxxxx, Yyyyyy (Twidd");
		case 4: return qApp->translate ("FlightModel", "Xxxxxxxx, Yyyyyy (Twidd");
		case 5: return qApp->translate ("FlightModel", "Launch method"); // Header text is longer than content
		// Improvement: use QStyle::PM_ButtonMargin for buttons
		case 6: return qApp->translate ("FlightModel", "  Depart  ");
		case 7: return qApp->translate ("FlightModel", "  Land  ");
		case 8: return qApp->translate ("FlightModel", "00:00");
		case 9: return qApp->translate ("FlightModel", "Ldgs."); // Header text is longer than content
		case 10: return qApp->translate ("FlightModel", "Twiddlethorpe");
		case 11: return qApp->translate ("FlightModel", "Twiddlethorpe");
		case 12: return qApp->translate ("FlightModel", "Cable break training");
		case 13: return qApp->translate ("FlightModel", "Landing fee paid");
		case 14: return qApp->translate ("FlightModel", "12/34/5678");
        case 15: return notr("424242");
        case 16: return qApp->translate ("FlightModel", "12345");
        case 17: return notr ("ABCDEF");
	}

	assert (!notr ("Unhandled column"));
	return QString ();
}

QVariant FlightModel::data (const Flight &flight, int column, int role) const
{
    Settings& s = Settings::instance();
	// TODO more caching - this is called very often
	// TODO isButtonRole and buttonTextRole should be in xxxData ()

    if (role==Qt::DisplayRole || role==csvExportRole)
	{
		switch (column)
		{
			case 0: return registrationData (flight, role);
			case 1: return planeTypeData (flight, role);
			case 2: return Flight::shortTypeText (flight.getType ());
            case 3: return s.anonymousMode ? flight.getNumCrew() : pilotData (flight, role);
            case 4: return s.anonymousMode ? flight.getNumPax() : copilotData (flight, role);
			case 5: return launchMethodData (flight, role);
			case 6: return departureTimeData (flight, role);
			case 7: return landingTimeData (flight, role);
			case 8: return durationData (flight, role);
			case 9: return flight.getNumLandings ();
			case 10: return flight.getDepartureLocation ();
			case 11: return flight.getLandingLocation ();
			case 12: return flight.getComments ();
			case 13: return flight.getAccountingNotes ();
            case 14: return flight.effdatum();
            case 15: return (flight.getVfId() == 0) ?
                            qApp->translate ("FlightModel", "No") :
                            qApp->translate ("FlightModel", "Yes");
            case 16: return (flight.isTowflight ()?qnotr ("(%1)"):qnotr ("%1")).arg (flight.getId ());
            case 17: return flight.getFlarmId ();

		}

		assert (false);
		return QVariant ();
	}
	else if (role==Qt::BackgroundRole)
	{
		if (colorEnabled)
			return QBrush (flight.getColor (cache));
		else
			return QVariant ();
	}
	else if (role==isButtonRole)
	{
        if (!buttonsEnabled)
            return false;

		// Only show buttons for prepared flights and today's flights
		if (!flight.isPrepared() && flight.getDepartureTime ().toLocalTime ().date ()!=QDate::currentDate ())
			return false;

		if      (column==departButtonColumn ()) { return flight.canDepart (); }
		else if (column==  landButtonColumn ()) { return flight.canLand (); }
		else return false;
	}
	else if (role==buttonTextRole)
	{
		if (column==departButtonColumn ())
			return qApp->translate ("FlightModel", "Depart");
		else if (column==landButtonColumn ())
		{
			if (flight.isTowflight () && !flight.landsHere ())
				return qApp->translate ("FlightModel", "End");
			else
				return qApp->translate ("FlightModel", "Land");
		}
		return QVariant ();
	}

	return QVariant ();
}


// ******************
// ** Data methods **
// ******************

QVariant FlightModel::registrationData (const Flight &flight, int role) const
{
	(void)role;

	try
	{
		Plane plane=cache.getObject<Plane> (flight.getPlaneId ());
		return plane.fullRegistration ();
	}
	catch (Cache::NotFoundException &)
	{
		return notr ("???");
	}
}

QVariant FlightModel::planeTypeData (const Flight &flight, int role) const
{
	(void)role;

	try
	{
		Plane plane=cache.getObject<Plane> (flight.getPlaneId ());
		return plane.type;
	}
	catch (Cache::NotFoundException &)
	{
		return notr ("???");
	}
}

QVariant FlightModel::pilotData (const Flight &flight, int role) const
{
	(void)role;

	try
	{
		Person pilot=cache.getObject<Person> (flight.getPilotId ());
		return pilot.formalNameWithClub ();
	}
	catch (Cache::NotFoundException &)
	{
		return flight.incompletePilotName ();
	}
}

QVariant FlightModel::copilotData (const Flight &flight, int role) const
{
	(void)role;

    try {
        if (Flight::typeCopilotRecorded (flight.getType ())) {
			Person copilot=cache.getObject<Person> (flight.getCopilotId ());
			return copilot.formalNameWithClub ();
        } else if (Flight::typeSupervisorRecorded(flight.getType())) {
            Person supervisor=cache.getObject<Person> (flight.getSupervisorId ());
            return QString("(%1: %2)").arg(qApp->translate("FlightModel", "Supervisor")).arg(supervisor.lastName);
        } else if (Flight::typeIsGuest (flight.getType ())) {
			return qApp->translate ("FlightModel", "(Passenger)");
        } else {
			return notr ("-");
		}

    } catch (Cache::NotFoundException &) {
		return flight.incompleteCopilotName ();
	}
}

QVariant FlightModel::launchMethodData (const Flight &flight, int role) const
{
	(void)role;

	try
	{
		if (!flight.departsHere ()) return notr ("-");

		// For towflights without launch method, assume self launch
		if (idInvalid (flight.getLaunchMethodId ()) && flight.isTowflight ()) return qApp->translate ("FlightModel", "SL", "Self launch");

		LaunchMethod launchMethod=cache.getObject<LaunchMethod> (flight.getLaunchMethodId ());

		return launchMethod.shortName;

		// Alternative: if (launchMethod.is_airtow () && !launchMethod.towplaneKnown) return towplane.registraion or "???"
	}
	catch (Cache::NotFoundException &)
	{
		return notr ("?");
	}
}

QVariant FlightModel::departureTimeData (const Flight &flight, int role) const
{
	(void)role;

	if (!flight.canHaveDepartureTime ())
		return notr ("-");
	else if  (!flight.hasDepartureTime ())
		return "";
	else
		// Don't use the default (locale) formatting, it may add seconds or
		// time zone information
        if (role == csvExportRole) {
            return flight.getDepartureTime() .toUTC ().time ().toString (notr ("hh:mm"));
        } else {
            return flight.getDepartureTime() .toUTC ().time ().toString (notr ("hh:mm"))+notr ("Z");
        }
}

QVariant FlightModel::landingTimeData (const Flight &flight, int role) const
{
	(void)role;

	if (!flight.canHaveLandingTime())
		return notr ("-");
	else if (!flight.hasLandingTime ())
		return "";
	else
		// Don't use the default (locale) formatting, it may add seconds or
		// time zone information
        if (role == csvExportRole) {
            return flight.getLandingTime ().toUTC ().time ().toString (notr ("hh:mm"));
        } else {
            return flight.getLandingTime ().toUTC ().time ().toString (notr ("hh:mm"))+notr ("Z");
        }
}

QVariant FlightModel::durationData (const Flight &flight, int role) const
{
	(void)role;

	if (!flight.hasDuration ())
		return notr ("-");
	else
		// Don't use the default (locale) formatting, it may add time zone
		// information (which is inapproprate for a duration) and we don't want
		// the duration zero-padded.
		return flight.flightDuration ().toString (notr ("h:mm"));
}
