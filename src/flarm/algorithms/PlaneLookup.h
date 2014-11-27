#ifndef PLANELOOKUP_H_
#define PLANELOOKUP_H_

#include "src/db/dbId.h"
#include "src/container/Maybe.h"
#include "src/flarm/flarmNet/FlarmNetRecord.h"

class Cache;
class Plane;

/**
 * Provides methods for looking up a plane for a given Flarm ID
 *
 * Given a Flarm ID, a plane can be identified by different criteria, in
 * decreasing order or reliability:
 *   1. The Flarm ID matches the Flarm ID of the plane
 *   2. The Flarm ID matches the Flarm ID of a FlarmNet record whose
 *      registration matches the registration of a plane
 *
 * Usage:
 *   PlaneLookup::Result result=PlaneLookup (cache).lookupPlane (flarmId);
 *
 * Note that plane lookup is not the same thing as plane identification. The
 * difference is explained in the documentation of the PlaneIdentification
 * class.
 *
 * See also: the wiki page "Flarm handling".
 */
class PlaneLookup
{
	public:
		/**
		 * Represents the plane lookup result. Can contain a plane and/or a
		 * FlarmNet record.
		 */
		class Result
		{
			public:
				Maybe<Plane>          plane;
				Maybe<FlarmNetRecord> flarmNetRecord;

				Result (const Maybe<Plane> &plane, const Maybe<FlarmNetRecord> &flarmNetRecord):
					plane (plane), flarmNetRecord (flarmNetRecord)
				{
				}

				static Result nothing ()
				{
					return Result (NULL, NULL);
				}
		};

		PlaneLookup (Cache &cache);
		virtual ~PlaneLookup ();

		Result lookupPlane (const QString &flarmId);

	protected:
		Result lookupPlaneByFlarmId          (const QString &flarmId);
		Result lookupPlaneByFlarmNetDatabase (const QString &flarmId);

	private:
		Cache &cache;
};

#endif
