#ifndef FLIGHTLOOKUP_H_
#define FLIGHTLOOKUP_H_

#include <QString>

#include "src/db/dbId.h"
#include "src/container/Maybe.h"
#include "src/flarm/flarmNet/FlarmNetRecord.h"
#include "src/FlightReference.h"

class Cache;
class Flight;
class Plane;
class FlarmNetRecord;

/**
 * Provides methods for looking up a flight for a given Flarm ID
 *
 * Given a Flarm ID, a flight can be identified by different criteria, in
 * decreasing order of reliability:
 *   1. The Flarm ID matches the Flarm ID of the flight.
 *      This means that the flight was created automatically. This is the most
 *      reliable criterion as it does not rely on any user-entered data. This
 *      criterion can only fail if the Flarm ID changes during the flight.
 *   2. The Flarm ID matches the Flarm ID of the plane of the flight. This is
 *      the typical case for prepared flights of known planes.
 *   3. The Flarm ID matches the Flarm ID of a FlarmNet record whose
 *      registration matches the registration of the plane of the flight.
 *      This is the least reliable criterion because the FlarmNet database
 *      itself may be inaccurate, and our copy of the FlarmNet database may be
 *      outdated.
 *
 * Note that this class does not specifically handle towflights. If you want to
 * include towflights as lookup candidates, you must include them in the
 * candidate list  passed to lookupFlights (see Flight::makeTowflight and
 * Flight::makeTowflights).
 *
 * See also: the wiki page "Flarm handling".
 */
class FlightLookup
{
	public:
		class Result
		{
			public:
				FlightReference       flightReference;
				Maybe<Plane>          plane;
				Maybe<FlarmNetRecord> flarmNetRecord;

				Result (const FlightReference flightReference,
					const Maybe<Plane> &plane,
					const Maybe<FlarmNetRecord> &flarmNetRecord):
					flightReference (flightReference), plane (plane),
					flarmNetRecord (flarmNetRecord)
				{
				}

				static Result nothing ()
				{
					return Result (FlightReference::invalid, NULL, NULL);
				}
		};

		FlightLookup (Cache &cache);
		virtual ~FlightLookup ();

		Result lookupFlight (const QList<Flight> &candidates, const QString &flarmId);

	protected:
		Result lookupFlightByFlarmId (const QList<Flight> &candidates, const QString &flarmId);
		Result lookupFlightByPlane   (const QList<Flight> &candidates, const QString &flarmId);

	private:
		Cache &cache;
};

#endif
