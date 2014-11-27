#ifndef PLANEIDENTIFICATION_H_
#define PLANEIDENTIFICATION_H_

#include <src/db/dbId.h>

class QWidget;
class DbManager;
class Plane;
class FlarmNetRecord;
class Flight;

/**
 * A helper class for interactively identifying the plane of a flight
 *
 * Note that plane identification is not the same as plane lookup (done by the
 * PlaneLookup class): plane lookup finds the plane for a given Flarm ID,
 * whereas plane identification finds the plane for a given flight. In the best
 * case, this consists of just one plane lookup, but it may also involve
 * letting the user choose from several candidates or even creating a new plane.
 * Plane lookup, on the other hand, is non-interactive.
 *
 * Note that this class does not actually update the flight in the database.
 * This is the responsibility of the caller. The reason is that some callers may
 * not want the flight updated immediately, for example, when editing a flight.
 *
 * See also: the wiki page "Flarm handling".
 */
class PlaneIdentification
{
	public:
		PlaneIdentification (DbManager &dbManager, QWidget *parent);
		virtual ~PlaneIdentification ();

		dbId interactiveIdentifyPlane (const Flight &flight, bool manualOperation);

	protected:
		void notCreatedAutomaticallyMessage ();
		void notCurrentMessage ();
		void identificationFailureMessage ();
		void currentMessage ();

		bool queryUsePlane (const Plane &plane, const Flight &flight);
		bool queryCreatePlane (const FlarmNetRecord &flarmNetRecord);

		dbId interactiveCreatePlane (const FlarmNetRecord &flarmNetRecord);

	private:
		DbManager &dbManager;
		QWidget *parent;

		bool manualOperation;
};

#endif
