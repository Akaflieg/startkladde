#ifndef FLARMIDCHECK_H_
#define FLARMIDCHECK_H_

#include <QObject>
#include <QList>
#include <QString>

#include "src/model/Plane.h"
#include "src/container/Maybe.h"

class DbManager;
class QWidget;

/**
 * A helper class for checking for and resolving Flarm ID conflicts when
 * updating a plane
 *
 * The Flarm ID of a plane should be unique, that is, there should be only one
 * plane with a given Flarm ID at a time. However, Flarm IDs may change as Flarm
 * units are moved around between planes. When creating or updating a plane, we
 * want to check for a conflict and offer the user some choices to resolve the
 * conflict. The selection of possible resolutions is non-trivial; additionally,
 * this has to be done in several places. This class contains the required
 * functionality.
 *
 * When updating a plane, create an instance of FlarmIdCheck. Call the
 * interactiveCheck method, passing the new Flarm ID, the old Flarm ID and the
 * plane ID. This will determine if there is a conflict and, if so, ask the user
 * for a resolution.
 *
 * You can then proceed to perform other checks, if required. When finished,
 * call the interactiveApply method of the FlarmIdCheck instance. This will
 * perform all the required changes to the database (allowing the user to
 * cancel, therefore "interactive"). The plane's Flarm ID may have to be changed
 * as well (e. g. if the user chooses to swap the Flarm ID with the conflicting
 * plane), so a pointer to it must be passed to interactiveApply.
 *
 * Both interactiveCheck and interactiveApply will return false if the user
 * cancels and true otherwise. If the user canceled during interactiveCheck,
 * interactiveApply will do nothing, so you may choose to ignore the return
 * value of interactiveCheck if you are going to call interactiveApply right
 * away.
 *
 * Note that the case of multiple conflicting planes are not handled at the
 * moment. In this case, one of them will be arbitrarily selected and treated as
 * it was the only one. Even after resolving the conflict, there will still be
 * multiple planes with the same Flarm ID.
 */
class FlarmIdCheck: public QObject
{
		Q_OBJECT

	public:
		FlarmIdCheck (DbManager &dbManager, QWidget *parent);
		virtual ~FlarmIdCheck ();

		bool interactiveCheck (const QString &newFlarmId, dbId planeId, const QString &oldFlarmId);
		bool interactiveApply (QString *planeFlarmId);

		bool keepOldFlarmId ();

	protected:
		/** The resolution of a conflict */
		// TODO clearThis might be useful (and rename clear to clearOther)
		enum Resolution
		{
			clear,  // Set the other plane's Flarm ID to ""
			swap,   // Set the other plane's Flarm ID this plane's old Flarm ID
			keep,   // Keep this plane's old Flarm ID
			ignore, // Ignore the conflict (both planes will have the same Flarm ID)
			cancel  // Cancel the whole operation
		};

		/**
		 * A set of resolutions presented to a user in case of a conflict. The
		 * set of resolutions as wells as the default (preselected) resolution
		 * depend on the details of the conflict.
		 */
		class ResolutionSet
		{
			public:
				QList<Resolution> resolutions;
				Resolution defaultResolution;
				int getDefaultIndex () const
					{ return resolutions.indexOf (defaultResolution); }
		};

		Maybe<Plane> findConflictingPlane ();
		ResolutionSet getPossibleResolutions ();
		QString makeText (Resolution resolution);
		Resolution showChoiceDialog (const ResolutionSet &possibleResolutions);

	private:
		DbManager &dbManager;
		QWidget *parent;

		// Input data
		QString newFlarmId;
		dbId planeId;
		QString oldFlarmId;

		// Status
		Maybe<Plane> conflictingPlane; // The conflicting plane if there is a conflict
		Resolution selectedResolution; // The resolution chosen by the user

};

#endif
