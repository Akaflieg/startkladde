#include "FlarmIdCheck.h"

#include "src/db/DbManager.h"
#include "src/model/Plane.h"
#include "src/gui/windows/input/ChoiceDialog.h"
#include "src/concurrent/monitor/OperationCanceledException.h"
#include "src/text.h"


// The conditions for a conflict are:
//   * the new Flarm ID is not empty (new!="")
//   * the other plane's Flarm ID is identical to the new Flarm ID (other==new)

// The following conflict resolutions are available:
//   * clear - other = ""
//   * swap  - other = old
//   * keep  - new   = old
//   * ignore
//   * cancel

// The useful resolutions depend on the conditions: the old Flarm ID may be
// empty or non-empty. The new Flarm ID may be the same as or different from the
// old Flarm ID.
//
// Ignore and cancel are always available. Other resolutions must either change
// new or other to remove the conflict.
//
// This is only relevant for conflicts: new==other, new!=""
//
//   Old Flarm ID | New/other Flarm ID | Comments/Resolutions
//   -------------+--------------------+-----------------------
//   old==""      | new==old           | No conflict (new == "")
//                |                    |
//   old==""      | new!=old           | Flarm ID added
//                |                    | clear  -  other=""
//                |                    | keep   -  new  =old  -  keep empty value
//                |                    | (swap is the same as clear)
//                |                    |
//   old!=""      | new==old           | Flarm ID not changed
//                |                    | clear  -  other=""
//                |                    | (swap is the same as ignore)
//                |                    | (keep is the same as ignore)
//                |                    |
//   old!=""      | new!=old           | Flarm ID changed
//                |                    | clear  -  other=""
//                |                    | swap   -  other=old
//                |                    | keep   -  new  =old

// TODO: there may be multiple conflicting planes
// TODO: the old Flarm ID may also be a duplicate
// TODO: instead of, or in addition to, saying "this plane", we should show the
// registration

/**
 * Creates a FlarmIdCheck
 *
 * @param dbManager the database to access for reading and writing
 * @param parent the Qt parent widget for dialogs shown to the user
 */
FlarmIdCheck::FlarmIdCheck (DbManager &dbManager, QWidget *parent):
	dbManager (dbManager), parent (parent),
	planeId (invalidId), selectedResolution (ignore)
{
}

FlarmIdCheck::~FlarmIdCheck ()
{
}

/**
 * Checks the database to see if there is a Flarm ID conflict
 *
 * A conflicting plane is a plane that has the same Flarm ID as the Flarm ID
 * we're trying to set (i. e. the new Flarm ID) and a different Flarm ID than
 * the plane we're editing. The latter restriction is required because when
 * updating an existing plane and *not* changing the Flarm ID, we still want to
 * check for conflicts, but we will always find the same plane when looking for
 * a plane with that Flarm ID.
 *
 * @return a Plane if a conflict was found, or an invalid Maybe if not
 */
Maybe<Plane> FlarmIdCheck::findConflictingPlane ()
{
	Cache &cache=dbManager.getCache ();

	// If the new Flarm ID is empty, there is no conflict
	if (isBlank (newFlarmId))
		return NULL;

	// Find a list of conflicting plane IDs. Note that this may also include
	// the same flight (if the Flarm ID was not changed), which is not actually
	// a conflict.
	QList<dbId> conflictingPlaneIds=cache.getPlaneIdsByFlarmId (newFlarmId);

	// If there are no planes with that Flarm ID, there is no conflict.
	if (conflictingPlaneIds.isEmpty ())
		return NULL;

	// If any of the planes we found is actually a conflict (i. e., not the one
	// we're editing), retrieve it from the database and return it.
	foreach (dbId id, conflictingPlaneIds)
	{
		// Make sure it is different from the plane we're editing.
		if (id!=planeId)
		{
			try
			{
				// Retrieve the plane
				return cache.getObject<Plane> (id);
			}
			catch (Cache::NotFoundException &)
			{
				// This should never happen because we just got the ID from the
				// database.
			}
		}
	}

	// We didn't find a conflicting plane (the only plane with that Flarm ID is
	// the plane we're editing).
	return NULL;
}

/**
 * Determines the possible resolutions to a conflict and suggests a default
 *
 * The useful resolution depend on several factors, such as whether the Flarm ID
 * actually changed and whether the Flarm ID was set before.
 *
 * This method does not check if there is a conflict. Its result is only
 * meaningful if there is.
 *
 * @return a ResolutionSet instance, which contains a list of possible
 *         resolutions and a default resolution
 */
FlarmIdCheck::ResolutionSet FlarmIdCheck::getPossibleResolutions ()
{
	// cancel is never returned. It is not explicitly offered along with the
	// other resolutions but represents the user canceling the choice dialog.

	ResolutionSet result;

	// For the resolutions available in different situations, see the comments
	// at the beginning of the file.

	if (newFlarmId!=oldFlarmId)
	{
		// The Flarm ID changed. The possible
		// resolutions are:
		//   * clear other
		//   * swap with other
		//   * keep
		//   * ignore
		result.resolutions << clear;
		// If the old Flarm ID is empty, swap would be the same as clear.
		if (!oldFlarmId.isEmpty ())
			result.resolutions << swap;
		result.resolutions << keep;
		result.resolutions << ignore;
		result.defaultResolution=clear;
	}
	else
	{
		// The Flarm ID did not change (new==old==other!=""). This limits the
		// useful resolutions to:
		//   * clear other     (this=new, other="")
		//   * ignore          (this=new, other=other)
		result.resolutions << clear;
		result.resolutions << ignore;
		result.defaultResolution=ignore;
	}

	return result;
}

/**
 * Creates a textual representation for a (possible) resolution for displying to
 * the user. This depends heavily on the current status.
 */
QString FlarmIdCheck::makeText (Resolution resolution)
{
	(void)newFlarmId;

	// No conflict
	if (!conflictingPlane.isValid ())
		return "";

	switch (resolution)
	{
		case clear:
			return tr ("Clear the Flarm ID of %1")
				.arg (conflictingPlane->registration);
		case swap:
			return tr ("Swap the Flarm ID with %1")
				.arg (conflictingPlane->registration);
		case ignore:
			return tr ("Ignore the conflict - automatic departures and "
				"landings may not work correctly");
		case keep:
			// The old Flarm ID may be empty. In this case, we need a different
			// message.
			if (oldFlarmId.isEmpty ())
				return tr ("Don't set a Flarm ID for this plane");
			else
				return tr ("Keep this plane's Flarm ID at its old value, %1")
					.arg (oldFlarmId);
		case cancel:
			// Not actually used
			return tr ("Cancel");

		// No default
	}

	return "";
}

/**
 * Displays a choice dialog to the user and returns the chosen resolution
 *
 * The available resolutions and the default resolution are passed in the
 * possibleResolutions argument.
 *
 * If the user cancels, cancel will be returned. Otherwise, the selected
 * resolution will be returned.
 */
FlarmIdCheck::Resolution FlarmIdCheck::showChoiceDialog (const ResolutionSet &possibleResolutions)
{
	if (!conflictingPlane.isValid ())
		return keep;

	// Choice dialog title and text
	// TODO this says "The entered Flarm ID", even though in some cases, the
	// Flarm ID may not have been explicitly entered explicitly (e. g. when
	// performing a Flarm ID update on saving a flight when the plane was
	// changed).
	QString title=tr ("Flarm ID conflict");
	QString text=tr (
		"<html><b>The entered Flarm ID, %1, is already used for the plane %2.</b></html>")
		.arg (newFlarmId)
		.arg (conflictingPlane->registrationWithType ());

	// Make a list of options
	QStringList options;
	foreach (Resolution resolution, possibleResolutions.resolutions)
		options << makeText (resolution);

	// Determine the default option index
	int defaultOptionIndex=possibleResolutions.getDefaultIndex ();

	// Show the dialog to the user
	int chosenIndex=ChoiceDialog::choose (title, text, options, defaultOptionIndex, parent);

	// Determine the solution from the user's choice
	if (chosenIndex<0)
		return cancel;
	else
		return possibleResolutions.resolutions[chosenIndex];
}

/**
 * Checks for a conflict and asks the user for a resolution
 *
 * If there is no conflict, nothing is done.
 *
 * @param newFlarmId the new Flarm ID value (may or may not be different from
 *                   the old value)
 * @param oldFlarmId the old Flarm ID value (as it currently is in the database)
 * @return true if OK, false if canceled
 */
bool FlarmIdCheck::interactiveCheck (const QString &newFlarmId, dbId planeId, const QString &oldFlarmId)
{
	this->newFlarmId=newFlarmId;
	this->planeId   =planeId;
	this->oldFlarmId=oldFlarmId;

	conflictingPlane.clearValue ();

	// Find a conflicting plane, i. e. a plane that alrady has the Flarm ID
	// we're trying to set, unless the new Flarm ID is blank.
	conflictingPlane=findConflictingPlane ();
	if (!conflictingPlane.isValid ())
		return true;

	// Determine the available resolutions
	ResolutionSet possibleResolutions=getPossibleResolutions ();

	selectedResolution=showChoiceDialog (possibleResolutions);

	if (selectedResolution==cancel)
		return false;
	else
		return true;
}

/**
 * Applies the resolution selected by the user
 *
 * This method may only be called after calling interactiveCheck, but regardless
 * of the return value of interactiveCheck.
 *
 * If the other plane has to be changed, the database is updated.
 *
 * Unless the user canceled, the plane's new Flarm ID is written via the pointer
 * passed as an argument (unless the pointer is NULL). Depending on whether
 * there was a conflict and on the user's choice, this may be the new or the old
 * Flarm ID.
 *
 * @param flightFlarmId a pointer to the plane's Flarm ID. This should not be
 *                      NULL as the value may have to be changed.
 * @return false if the user canceled or if the user had canceled during the
 *               preceding call to interactiveCheck, or true otherwise
 */
bool FlarmIdCheck::interactiveApply (QString *planeFlarmId)
{
	Maybe<QString> thisFlightFlarmId;
	Maybe<QString> otherFlightFlarmId;
	bool canceled=false;

	if (!conflictingPlane.isValid ())
	{
		// There was no conflict.
		thisFlightFlarmId.setValue (newFlarmId);
	}
	else
	{
		// There was a conflict. Let's see what the user chose to do.
		switch (selectedResolution)
		{
			case clear:
				thisFlightFlarmId.setValue (newFlarmId);
				otherFlightFlarmId.setValue ("");
				break;
			case swap:
				thisFlightFlarmId.setValue (newFlarmId);
				otherFlightFlarmId.setValue (oldFlarmId);
				break;
			case keep:
				thisFlightFlarmId.setValue (oldFlarmId);
				break;
			case ignore:
				thisFlightFlarmId.setValue (newFlarmId);
				break;
			case cancel:
				canceled=true;
				break;
			// No default
		}
	}

	if (thisFlightFlarmId.isValid ())
	{
		if (planeFlarmId)
			(*planeFlarmId)=thisFlightFlarmId.getValue ();
	}

	if (otherFlightFlarmId.isValid ())
	{
		try
		{
			conflictingPlane->flarmId=otherFlightFlarmId.getValue ();
			dbManager.updateObject (conflictingPlane.getValue (), parent);
		}
		catch (OperationCanceledException &)
		{
			return false;
		}
	}

	return !canceled;
}
