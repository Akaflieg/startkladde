#include "PlaneLookup.h"

#include <iostream>

#include "src/db/cache/Cache.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/flarm/flarmNet/FlarmNetRecord.h"
#include "src/i18n/notr.h"
#include "src/config/Settings.h"

// TODO handle properly that case that one lookup source returns multiple
// results (e. g. multiple planes with that Flarm ID, lookupPlaneByFlarmId)

PlaneLookup::PlaneLookup (Cache &cache):
	cache (cache)
{
}

PlaneLookup::~PlaneLookup ()
{
}

/**
 * Tries to find a plane with the given Flarm ID
 *
 * The result can contain:
 *
 * Plane | FNR | Description
 * ------+-----+-------------------
 * yes   | no  | plane was found
 * ------+-----+-------------------
 * no    | no  | no plane was found
 */
PlaneLookup::Result PlaneLookup::lookupPlaneByFlarmId (const QString &flarmId)
{
	try
	{
		// Find a plane with that Flarm ID
		dbId planeId=cache.getPlaneIdByFlarmId (flarmId);

		if (idValid (planeId))
		{
			// Found a plane. Retrieve it from the database.
			Plane plane=cache.getObject<Plane> (planeId);
			return Result (plane, NULL);
		}
	}
	catch (Cache::NotFoundException &ex)
	{
	}

	return Result::nothing ();
}

/**
 * Tries to find a FlarmNet record with the given Flarm ID, and, on success, a
 * plane with the registration from the FlarmNet record
 *
 * The result can contain:
 *
 * Plane | FNR | Description
 * ------+-----+-----------------------------------------
 * yes   | yes | plane was found via FlarmNet
 * ------+-----+-----------------------------------------
 * no    | yes | FlarmNet record, but no plane, was found
 * ------+-----+-----------------------------------------
 * no    | no  | nothing was found
 *
 * If a plane was found via FlarmNet but its Flarm ID does not match the Flarm
 * ID we're looking for, nothing is returned.
 */
PlaneLookup::Result PlaneLookup::lookupPlaneByFlarmNetDatabase (const QString &flarmId)
{
	try
	{
		// Find a FlarmNet record with that Flarm ID.
		dbId flarmNetRecordId = cache.getFlarmNetRecordIdByFlarmId (flarmId);

		// Return nothing if there is none.
		if (!idValid (flarmNetRecordId))
			return Result::nothing ();

		// Found a FlarmNet record. Retrieve it from the database.
		FlarmNetRecord flarmNetRecord = cache.getObject<FlarmNetRecord> (flarmNetRecordId);

		// Return nothing if the FlarmNet record's registration is empty.
		if (flarmNetRecord.registration.isEmpty ())
			return Result::nothing ();

		// Find a plane with the registration from the FlarmNet record.
		dbId planeId=cache.getPlaneIdByRegistration (flarmNetRecord.registration);

		// Return just the FlarmNet record if there is none.
		if (!idValid (planeId))
			return Result (NULL, flarmNetRecord);

		// Found a plane. Retrieve it from the database.
		Plane plane=cache.getObject<Plane> (planeId);

		// If the plane has a Flarm ID and it's different from the one we're
		// looking for, assume that the FlarmNet record is outdated and ignore
		// it (i. e. don't even return the FlarmNet record).
		if (!plane.flarmId.isEmpty () && plane.flarmId!=flarmId)
			return Result::nothing ();

		// The plane's Flarm ID is either empty or matches the one we're looking
		// for, so we finally found the plane.
		return Result (plane, flarmNetRecord);
	}
	catch (Cache::NotFoundException &)
	{
		// This should not happen. But actually, if plane lookup fails, we might
		// still return the FlarmNet record. We won't change that now because
		// exception will be removed anyway.
	}

	return Result::nothing ();
}

/**
 * Tries to find a plane for a given Flarm ID, by any means supported and
 * enabled in the configuration
 *
 * The result can contain:
 *
 * Plane | FNR | Description
 * ------+-----+-------------------------------------------
 * yes   | no  | plane found directly
 * yes   | yes | plane found via FlarmNet
 * ------+-----+-------------------------------------------
 * no    | yes | a FlarmNet record, but no plane, was found
 * ------+-----+-------------------------------------------
 * no    | no  | nothing was found
 *
 * If the result contains a plane, it is guaranteed to have either no Flarm ID
 * or the Flarm ID we're looking for.
 */
PlaneLookup::Result PlaneLookup::lookupPlane (const QString &flarmId)
{
	Result result=Result::nothing ();

	// Try to find the plane by Flarm ID
	result=lookupPlaneByFlarmId (flarmId);

	// If we found a plane, return the result
	if (result.plane.isValid ())
		return result;

	// Try to find the plane via FlarmNet database if FlarmNet is enabled
	if (Settings::instance ().flarmNetEnabled)
	{
		result=lookupPlaneByFlarmNetDatabase (flarmId);

		// If we found anything, return the result
		if (result.plane.isValid ())
			return result;
		else if (result.flarmNetRecord.isValid ())
			return result;
	}

	// None of the criteria matched, and no useful data was returned.
	return Result::nothing ();
}
