#ifndef FLARMIDUPDATE_H_
#define FLARMIDUPDATE_H_

#include <QMessageBox>

#include "src/db/dbId.h"

class DbManager;
class QWidget;
class Flight;
class Plane;

/**
 * A helper class for interactively updating the Flarm ID of a plane in the
 * database
 *
 * Note that this class does update the plane in the database, as opposed to
 * PlaneIdentification, which only determines what to do and leaves the exection
 * to the caller.
 *
 * See also: the wiki page "Flarm handling".
 */
class FlarmIdUpdate
{
	public:
		FlarmIdUpdate (DbManager &dbManager, QWidget *parent);
		virtual ~FlarmIdUpdate ();

		bool interactiveUpdateFlarmId (const Flight &flight, bool manualOperation, dbId oldPlaneId);

	protected:
		enum UpdateAction { update, dontUpdate, cancel };

		void notCreatedAutomaticallyMessage ();
		void notCurrentMessage ();
		void noPlaneMessage ();
		void currentMessage ();

		UpdateAction queryUpdateFlarmId (const Plane &plane, const Flight &flight);

		bool canUpdateSilently (const Plane &plane, const Flight &flight);
		bool checkAndUpdate (Plane &plane, const Flight &flight);


	private:
		DbManager &dbManager;
		QWidget *parent;

		bool manualOperation;
		dbId oldPlaneId;
};

#endif
