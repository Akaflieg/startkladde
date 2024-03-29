#include "Flight.h"

#include <cassert>

#include <QApplication>
#include <QList>

#include "src/i18n/notr.h"

QList<Flight::Type> Flight::listTypes (bool includeInvalid)
{
	if (includeInvalid)
		return listTypes (false) << typeNone;
	else
        return QList<Type> ()
                   << typeNormal << typeTraining2 << typeTraining1 << typeAuffrischung
			<< typeGuestPrivate << typeGuestExternal;
}

QString Flight::typeText (Type type, bool withShortcut)
{
	if (withShortcut)
	{
		switch (type)
		{
			case typeNone:          return notr ("-");
			case typeNormal:        return qApp->translate ("Flight::Type", "R - Regular flight");
			case typeTraining2:     return qApp->translate ("Flight::Type", "2 - Two-seated training");
			case typeTraining1:     return qApp->translate ("Flight::Type", "1 - Solo training");
            case typeAuffrischung:  return qApp->translate ("Flight::Type", "Ü - Two-seated refresher training");
			case typeTow:           return qApp->translate ("Flight::Type", "T - Towflight");
			case typeGuestPrivate:  return qApp->translate ("Flight::Type", "P - Passenger flight");
			case typeGuestExternal: return qApp->translate ("Flight::Type", "E - Passenger flight (extern)");
			// No default
		}
	}
	else
	{
		switch (type)
		{
			case typeNone:          return notr ("-");
            case typeNormal:        return qApp->translate ("Flight::Type", "Regular flight");
            case typeTraining2:     return qApp->translate ("Flight::Type", "Two-seated training");
            case typeTraining1:     return qApp->translate ("Flight::Type", "Solo training");
            case typeAuffrischung:  return qApp->translate ("Flight::Type", "Two-seated refresher training");
			case typeTow:           return qApp->translate ("Flight::Type", "Towflight");
            case typeGuestPrivate:  return qApp->translate ("Flight::Type", "Passenger flight");
            case typeGuestExternal: return qApp->translate ("Flight::Type", "Passenger flight (external)");
			// No default
		}
	}

	assert (!notr ("Unhandled type"));
	return notr ("?");
}

QString Flight::shortTypeText (Type type)
{
	switch (type)
	{
		case typeNone:          return notr ("-");
		case typeNormal:        return qApp->translate ("Flight::Type", "Regular");
		case typeTraining2:     return qApp->translate ("Flight::Type", "Training (2)");
		case typeTraining1:     return qApp->translate ("Flight::Type", "Training (1)");
        case typeAuffrischung:  return qApp->translate ("Flight::Type", "Refresher training (Ü)");
		case typeTow:           return qApp->translate ("Flight::Type", "Tow");
		case typeGuestPrivate:  return qApp->translate ("Flight::Type", "Passenger");
		case typeGuestExternal: return qApp->translate ("Flight::Type", "Passenger (E)");
		// No default
	}

	assert (!notr ("Unhandled type"));
	return notr ("?");
}

// TODO rename allowed
bool Flight::typeCopilotRecorded (Flight::Type type)
{
	switch (type)
	{
		case typeNone: return true;
		case typeNormal: return true;
		case typeTraining2: return true;
        case typeTraining1: return false;
        case typeAuffrischung: return true;
		case typeTow: return true;
		case typeGuestPrivate: return false;
		case typeGuestExternal: return false;
	}

	assert (false);
	return false;
}

bool Flight::typeSupervisorRecorded (Flight::Type type)
{
    return type == typeTraining1;
}

bool Flight::typeAlwaysHasCopilot (Flight::Type type)
{
	switch (type)
	{
		case typeNone: return false;
		case typeNormal: return false;
		case typeTraining2: return true;
		case typeTraining1: return false;
        case typeAuffrischung: return true;
		case typeTow: return false;
		case typeGuestPrivate: return true;
		case typeGuestExternal: return true;
	}

	assert (false);
	return false;
}

QString Flight::typePilotDescription (Flight::Type type)
{
	switch (type)
	{
		case typeTraining2:
		case typeTraining1:
			return qApp->translate ("Flight::Type", "student");
        case typeAuffrischung:
            return qApp->translate("Flight::Type", "pilot being checked");
		case typeNone:
		case typeNormal:
		case typeGuestPrivate:
		case typeGuestExternal:
			return qApp->translate ("Flight::Type", "pilot");
		case typeTow:
			return qApp->translate ("Flight::Type", "towpilot");
	}

	assert (false);
	return qApp->translate ("Flight::Type", "pilot");
}

QString Flight::typeCopilotDescription (Flight::Type type)
{
	switch (type)
	{
		case typeTraining2:
			return qApp->translate ("Flight::Type", "flight instructor");
        case typeAuffrischung:
            return qApp->translate("Flight::Type", "checking flight instructor");
		case typeGuestPrivate:
		case typeGuestExternal:
			return qApp->translate ("Flight::Type", "passenger");
		case typeNone:
		case typeNormal:
		case typeTow:
			return qApp->translate ("Flight::Type", "copilot");
		case typeTraining1:
            return qApp->translate("Flight::Type", "supervising flight instructor");
	}

	assert (false);
	return qApp->translate ("Flight::Type", "copilot");
}

bool Flight::typeIsGuest (Flight::Type type)
{
	switch (type)
	{
		case typeNone: return false;
		case typeNormal: return false;
		case typeTraining2: return false;
        case typeTraining1: return false;
        case typeAuffrischung: return false;
		case typeTow: return false;
		case typeGuestPrivate: return true;
		case typeGuestExternal: return true;
	}

	assert (false);
	return false;
}

bool Flight::typeIsTraining (Flight::Type type)
{
	switch (type)
	{
		case typeNone: return false;
		case typeNormal: return false;
		case typeTraining2: return true;
        case typeTraining1: return true;
        case typeAuffrischung: return true;
		case typeTow: return false;
		case typeGuestPrivate: return false;
		case typeGuestExternal: return false;
	}

	assert (false);
	return false;
}

QString Flight::flighttypeRemark(Flight::Type type) {
    switch (type) {
        case typeGuestPrivate: return qApp->translate("Flight::Type", "Pilot is charged");
        case typeGuestExternal: return qApp->translate("Flight::Type", "Guest pays guest flight fee");
        case typeAuffrischung: return qApp->translate("Flight::Type", "Flight type for the 2 refresher flights every 2 years with instructor according to SFCL.160");
        default: return QString();
    }
}
