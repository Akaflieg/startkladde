#include "Cache.h"

#include <QMultiHash>

#include "src/container/SortedSet_impl.h"

/*
 * Currently, the update methods call the removed and added methods. This may
 * or may not be optimal or even correct.
 *
 * Also, we don't remove entries that may still be valid, for example name
 * parts where there may be another person with the same name part. For the
 * SortedSets, this could be implemented by making the sets count the number of
 * times a value has been added; for the MultiHashes it would be a bit more
 * work.
 */

#include "src/concurrent/synchronized.h"
#include "src/model/Flight.h"
#include "src/model/LaunchMethod.h"
#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/flarm/flarmNet/FlarmNetRecord.h"
#include "src/text.h"

// All of these methods do not handle the by-id hashes
// These methods must be specialized before they are used.
// No default that does nothing to avoid forgetting one
// The individual clear methods do not clear hashes use for multiple
// object types (such as clubs which is used for people and planes)

// ********************
// ** Multiple types **
// ********************

void Cache::clearMultiTypeHashes ()
{
	synchronized (dataMutex)
	{
		clubs.clear ();
	}
}


// ************
// ** Planes **
// ************

template<> void Cache::clearHashes<Plane> ()
{
	synchronized (dataMutex)
	{
		planeTypes.clear ();
		planeRegistrations.clear ();
		planeIdsByRegistration.clear ();
		planeIdsByFlarmId.clear ();
		// clubs is used by multiple types
	}
}

template<> void Cache::updateHashesObjectAdded<Plane> (const Plane &plane)
{
	// All values inserted here should be removed in the corresponding
	// updateHashesObjectDeleted method, if possible; otherwise, care must be
	// taken not to insert a value multiple times if an object is deleted and
	// re-added.
	synchronized (dataMutex)
	{
		if (!isBlank (plane.type)) planeTypes.insert (plane.type);
		planeRegistrations.insert (plane.registration);
		planeIdsByRegistration.insert (plane.registration.toLower (), plane.getId ());
		planeIdsByFlarmId.insert (plane.flarmId, plane.getId ());
		if (!isBlank (plane.club)) clubs.insert (plane.club);

        // The exact position is not too important (just defines the sort order in flight wizard),
        // so just add at the end. However it is important that the plane is added, because
        // otherwise a newly added plane will not appear before restart or full refresh in flight wizard.
        planesSortedByUsage.prepend(plane);
	}
}

template<> void Cache::updateHashesObjectDeleted<Plane> (dbId id, const Plane *oldPlane)
{
	synchronized (dataMutex)
	{
		// Leave planeTypes
		if (oldPlane)
		{
			QString registration=oldPlane->registration;
			QString registrationLower=registration.toLower ();
			QString flarmId=oldPlane->flarmId;

			planeIdsByRegistration.remove (registrationLower, id);
			if (!planeIdsByRegistration.contains (registrationLower))
				planeRegistrations.remove (registration);

			planeIdsByFlarmId.remove (flarmId, id);

            for (int i = 0; i < planesSortedByUsage.size(); ) {
                if (planesSortedByUsage.at(i).getId() == oldPlane->getId()) {
                    planesSortedByUsage.removeAt(i);
                } else {
                    i++;
                }
            }
		}
		// Leave clubs
	}
}

template<> void Cache::updateHashesObjectUpdated<Plane> (const Plane &plane, const Plane *oldPlane)
{
	synchronized (dataMutex)
	{
		updateHashesObjectDeleted<Plane> (plane.getId (), oldPlane);
		updateHashesObjectAdded (plane);
	}
}

// ************
// ** People **
// ************


template<> void Cache::clearHashes<Person> ()
{
	synchronized (dataMutex)
	{
		personLastNames.clear ();
		personFirstNames.clear ();
		lastNamesByFirstName.clear ();
		firstNamesByLastName.clear ();
		personIdsByLastName.clear ();
		personIdsByFirstName.clear ();
		personIdsByName.clear ();
		// clubs is used by multiple types
	}
}

template<> void Cache::updateHashesObjectAdded<Person> (const Person &person)
{
	// All values inserted here should be removed in the corresponding
	// updateHashesObjectDeleted method, if possible; otherwise, care must be
	// taken not to insert a value multiple times if an object is deleted and
	// re-added.
	synchronized (dataMutex)
	{
		const QString &last =person.lastName ; QString lastLower =last .toLower ();
		const QString &first=person.firstName; QString firstLower=first.toLower ();
		dbId id=person.getId ();

		personLastNames.insert (last);
		personFirstNames.insert (first);
		// insertUnique: avoid duplicate identical (!) values for a given key
		lastNamesByFirstName.insertUnique (firstLower, last );
		firstNamesByLastName.insertUnique (lastLower , first);
		personIdsByLastName .insert (lastLower , id);
		personIdsByFirstName.insert (firstLower, id);
		personIdsByName.insert (QPair<QString, QString> (lastLower, firstLower), id);
		if (!isBlank (person.club)) clubs.insert (person.club);

        // The exact position is not too important (just defines the sort order in flight wizard),
        // so just add at the last positions. However it is important that the person is added, because
        // otherwise a newly added person will not appear before restart or full refresh in flight wizard.
        peopleSortedByFrequency.prepend(person);
	}
}

template<> void Cache::updateHashesObjectDeleted<Person> (dbId id, const Person *oldPerson)
{
	synchronized (dataMutex)
	{
		// Leave personLastNames
		// Leave personFirstNames
		// Leave lastNamesByFirstName
		// Leave firstNamesByLastName
		if (oldPerson)
		{
			QString lastLower =oldPerson-> lastName.toLower ();
			QString firstLower=oldPerson->firstName.toLower ();

			if (oldPerson) personIdsByLastName.remove (lastLower, id);
			if (oldPerson) personIdsByFirstName.remove (firstLower, id);
			if (oldPerson) personIdsByName.remove (QPair<QString, QString> (lastLower, firstLower), id);

            for (int i = 0; i < peopleSortedByFrequency.size(); ) {
                if (peopleSortedByFrequency.at(i).getId() == oldPerson->getId()) {
                    peopleSortedByFrequency.removeAt(i);
                } else {
                    i++;
                }
            }
		}
		// Leave clubs
	}
}

template<> void Cache::updateHashesObjectUpdated<Person> (const Person &person, const Person *oldPerson)
{
	synchronized (dataMutex)
	{
		updateHashesObjectDeleted<Person> (person.getId (), oldPerson);
		updateHashesObjectAdded (person);
	}
}


// ********************
// ** Launch methods **
// ********************

template<> void Cache::clearHashes<LaunchMethod> ()
{
	synchronized (dataMutex)
	{
		launchMethodIdsByType.clear ();
	}
}

template<> void Cache::updateHashesObjectAdded<LaunchMethod> (const LaunchMethod &launchMethod)
{
	// All values inserted here should be removed in the corresponding
	// updateHashesObjectDeleted method, if possible; otherwise, care must be
	// taken not to insert a value multiple times if an object is deleted and
	// re-added.
	synchronized (dataMutex)
	{
		launchMethodIdsByType.insert (launchMethod.type, launchMethod.getId ());
	}
}

template<> void Cache::updateHashesObjectDeleted<LaunchMethod> (dbId id, const LaunchMethod *oldLaunchMethod)
{
	synchronized (dataMutex)
	{
		if (oldLaunchMethod) launchMethodIdsByType.remove (oldLaunchMethod->type, id);
	}
}

template<> void Cache::updateHashesObjectUpdated<LaunchMethod> (const LaunchMethod &launchMethod, const LaunchMethod *oldLaunchMethod)
{
	synchronized (dataMutex)
	{
		updateHashesObjectDeleted<LaunchMethod> (launchMethod.getId (), oldLaunchMethod);
		updateHashesObjectAdded (launchMethod);
	}
}


// *************
// ** Flights **
// *************

template<> void Cache::clearHashes<Flight> ()
{
	synchronized (dataMutex)
	{
		locations.clear ();
		accountingNotes.clear ();
	}
}

template<> void Cache::updateHashesObjectAdded<Flight> (const Flight &flight)
{
	// All values inserted here should be removed in the corresponding
	// updateHashesObjectDeleted method, if possible; otherwise, care must be
	// taken not to insert a value multiple times if an object is deleted and
	// re-added.
	synchronized (dataMutex)
	{
		if (!isBlank (flight.       getDepartureLocation ())) locations      .insert (flight.       getDepartureLocation ());
		if (!isBlank (flight.         getLandingLocation ())) locations      .insert (flight.         getLandingLocation ());
		if (!isBlank (flight.getTowflightLandingLocation ())) locations      .insert (flight.getTowflightLandingLocation ());
		if (!isBlank (flight.         getAccountingNotes ())) accountingNotes.insert (flight.         getAccountingNotes ());
	}
}

template<> void Cache::updateHashesObjectDeleted<Flight> (dbId id, const Flight *oldFlight)
{
	synchronized (dataMutex)
	{
		(void)id;
		(void)oldFlight;
		// Leave locations (must include values from all flights)
		// Leave accountingNotes (must include values from all flights)
	}
}

template<> void Cache::updateHashesObjectUpdated<Flight> (const Flight &flight, const Flight *oldFlight)
{
	synchronized (dataMutex)
	{
		updateHashesObjectDeleted<Flight> (flight.getId (), oldFlight);
		updateHashesObjectAdded (flight);
	}
}

// ********************
// ** FlarmNetRecord **
// ********************
template<> void Cache::clearHashes<FlarmNetRecord> ()
{
	synchronized (dataMutex)
	{
		flarmNetRecordIdsByFlarmId.clear ();
	}
}

template<> void Cache::updateHashesObjectAdded<FlarmNetRecord> (const FlarmNetRecord &record)
{
	// All values inserted here should be removed in the corresponding
	// updateHashesObjectDeleted method, if possible; otherwise, care must be
	// taken not to insert a value multiple times if an object is deleted and
	// re-added.
	synchronized (dataMutex)
	{
		dbId id=record.getId ();
		QString flarmId = record.flarmId;

		flarmNetRecordIdsByFlarmId.insert (flarmId, id);
	}
}

template<> void Cache::updateHashesObjectDeleted<FlarmNetRecord> (dbId id, const FlarmNetRecord *oldRecord)
{
	synchronized (dataMutex)
	{
		QString flarmId = oldRecord->flarmId;

		if (flarmNetRecordIdsByFlarmId.contains (flarmId))
                        flarmNetRecordIdsByFlarmId.remove (flarmId, id);
	}
}

template<> void Cache::updateHashesObjectUpdated<FlarmNetRecord> (const FlarmNetRecord &record, const FlarmNetRecord *oldRecord)
{
	synchronized (dataMutex)
	{
		updateHashesObjectDeleted<FlarmNetRecord> (record.getId (), oldRecord);
		updateHashesObjectAdded (record);
	}
}

