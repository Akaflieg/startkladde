#include "PlaneIdentification.h"

#include <QWidget>

#include "src/gui/dialogs.h"
#include "src/db/DbManager.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/flarm/flarmNet/FlarmNetRecord.h"
#include "src/flarm/algorithms/PlaneLookup.h"
#include "src/gui/windows/objectEditor/ObjectEditorWindow.h"
#include "src/gui/windows/objectEditor/PlaneEditorPane.h"
#include "src/text.h"

/**
 * Creates a new PlaneIdentification instance
 *
 * The database manager to use and the parent for any dialogs we may need to
 * display to the user have to be specified.
 *
 * Note that this is not a QWidget; the parent widget will not delete this
 * instance.
 */
PlaneIdentification::PlaneIdentification (DbManager &dbManager, QWidget *parent):
	dbManager (dbManager), parent (parent),
	manualOperation (true)
{
}

PlaneIdentification::~PlaneIdentification ()
{
}

/**
 * Asks the user whether he wants to use the specified plane
 *
 * The flight also has to be specified so we can determine whether this will
 * just set the plane or overwrite an already-set value, which will be reflected
 * in the question(s).
 *
 * Returns true if the user chooses to use the plane, or false otherwise.
 */
bool PlaneIdentification::queryUsePlane (const Plane &plane, const Flight &flight)
{
	// Offer the user to use this plane

	QString title=qApp->translate ("PlaneIdentification", "Use plane?");

	// TODO test both cases
	QString text;
	if (isBlank (plane.type))
		text=qApp->translate ("PlaneIdentification",
			"The plane seems to be %1.")
			.arg (plane.fullRegistration ());
	else
		text=qApp->translate ("PlaneIdentification",
			"The plane seems to be a %1 with registration %2.")
			.arg (plane.type).arg (plane.fullRegistration ());

	if (idValid (flight.getPlaneId ()))
		text+=qApp->translate ("PlaneIdentification",
			" Do you want to replace the current plane with this plane?");
	else
		text+=qApp->translate ("PlaneIdentification",
			" Do you want to use this plane?");

	return yesNoQuestion (parent, title, text);
}

/**
 * Asks the user whether he wants to create a plane from the given FlarmNet
 * record
 *
 * Returns true if the user chooses to create the plane, or false otherwise
 */
bool PlaneIdentification::queryCreatePlane (const FlarmNetRecord &flarmNetRecord)
{
	// Offer the user to create a plane with the FlarmNet data
	QString title=qApp->translate ("PlaneIdentification", "Automatically create plane?");

	// TODO test both cases
	QString text;
	if (isBlank (flarmNetRecord.type))
		text=qApp->translate ("PlaneIdentification",
			"The plane was not found in the database. "
			"However, the FlarmNet database indicates that the registration of "
			"the plane might be %1. "
			"Do you want to create this plane?")
			.arg (flarmNetRecord.fullRegistration ());
	else
		text=qApp->translate ("PlaneIdentification",
			"The plane was not found in the database. "
			"However, the FlarmNet database indicates that the plane might be "
			"a %1 with registration %2. "
			"Do you want to create this plane?")
			.arg (flarmNetRecord.type)
			.arg (flarmNetRecord.fullRegistration ());

	return yesNoQuestion (parent, title, text);
}

/**
 * Shows a message to the user indicating that the plane cannot be identified
 * because the flight was not created automatically
 */
void PlaneIdentification::notCreatedAutomaticallyMessage ()
{
	QMessageBox::information (parent,
		qApp->translate ("PlaneIdentification", "Identify plane"),
		qApp->translate ("PlaneIdentification", "The plane cannot be "
			"identified because this flight was not created automatically."));
}

/**
 * Shows a message to the user indicating that the plane cannot be identified
 * because the flight is not current (i. e. of today)
 */
void PlaneIdentification::notCurrentMessage ()
{
	QMessageBox::information (parent,
		qApp->translate ("PlaneIdentification", "Identify plane"),
		qApp->translate ("PlaneIdentification", "The plane cannot be "
			"identified because this flight did not take place today."));
}

/**
 * Shows a message to the user indicating that the plane failed to identify
 */
void PlaneIdentification::identificationFailureMessage ()
{
	QMessageBox::information (parent,
		qApp->translate ("PlaneIdentification", "Identify plane"),
		qApp->translate ("PlaneIdentification", "The plane could not be identified."));
}

/**
 * Shows a message to the user indicating that the plane is already current
 */
void PlaneIdentification::currentMessage ()
{
	QMessageBox::information (parent,
		qApp->translate ("PlaneIdentification", "Identify plane"),
		qApp->translate ("PlaneIdentification", "The plane is already current."));
}

/**
 * Interactively creates a plane from the given FlarmNet record
 *
 * This will open a plane editor with the data from the FlarmNet record pre-
 * entered, let the user edit the values (except the Flarm ID) and create the
 * plane in the database if the user accepts.
 *
 * Returns the ID of the newly created plane, or an invalid ID if the user
 * cancels.
 */
dbId PlaneIdentification::interactiveCreatePlane (const FlarmNetRecord &flarmNetRecord)
{
	PlaneEditorPaneData paneData;
	paneData.flarmIdReadOnly=true;

	return ObjectEditorWindow<Plane>::createObjectPreset (parent,
		dbManager, flarmNetRecord.toPlane (true), &paneData, NULL);
}

/**
 * Tries to identify the plane for an automatically created flight, asking the
 * user to choose or create a plane if necessary
 *
 * This method returns a plane ID if a plane has been identified and selected or
 * created by the user. The plane ID may be the same as the flight already has.
 * If the user cancels, chooses not to use the identified plane, or
 * identification fails, an invalid ID is returned.
 *
 * If identification fails, a message is shown to the user if manualOperation is
 * true.
 *
 * The caller is responsible for setting the plane of the flight and updating
 * the flight in the database (if applicable) if the plane changed.
 *
 * The manualOperation parameter indicates whether the operation was invoked
 * explicitly by the user, as opposed to being triggered by some other action
 * like editing a flight. This implies that a message is always displayed, even
 * if nothing is done.
 */
dbId PlaneIdentification::interactiveIdentifyPlane (const Flight &flight, bool manualOperation)
{
	this->manualOperation=manualOperation;

	Cache &cache=dbManager.getCache ();

	try
	{
		// We can only do this for automatically created flights
		if (isBlank (flight.getFlarmId ()))
		{
			if (manualOperation)
				notCreatedAutomaticallyMessage ();
			return invalidId;
		}

		// We can only do this for current flights (flights of today)
		if (!flight.isCurrent ())
		{
			if (manualOperation)
				notCurrentMessage ();
			return invalidId;
		}

		// Look up the plane, and see what we've got
		PlaneLookup::Result result=PlaneLookup (cache).lookupPlane (flight.getFlarmId ());
		if (result.plane.isValid ())
		{
			// We found a plane.
			if (result.plane->getId ()==flight.getPlaneId ())
			{
				// The plane is already current
				currentMessage ();
			}
			else
			{
				// The plane is different. Offer the user to use it. Plane
				// lookup guarantees that the Flarm ID of a returned plane
				// matches. Therefore, we don't have to update its Flarm ID.
				if (queryUsePlane (result.plane.getValue (), flight))
					return result.plane->getId ();
				else
					return invalidId;
			}
		}
		else if (result.flarmNetRecord.isValid ())
		{
			// Offer the user to create a plane with the FlarmNet data
			// There is no plane with that Flarm ID, or plane lookup would have
			// found it. Therefore, we don't have to check for a Flarm ID
			// conflict.
			if (queryCreatePlane (result.flarmNetRecord.getValue ()))
				return interactiveCreatePlane (result.flarmNetRecord.getValue ());
			else
				return invalidId;
		}
		else
		{
			// Identification failed
			if (manualOperation)
				identificationFailureMessage ();

			return invalidId;
		}
	}
	catch (Cache::NotFoundException &ex)
	{
		return invalidId;
	}

	return invalidId;
}
