#ifndef FLIGHTREFERENCE_H_
#define FLIGHTREFERENCE_H_

#include <QString>

#include "src/db/dbId.h"

class Flight;

/**
 * Since a flight and - if it uses an airtow launch method - its towflight are
 * stored in a single database entry with a single ID, we need a way to refer to
 * one of them. This class represents a reference to either the flight proper or
 * the towflight with a given flight ID.
 *
 * A flight reference consists of a database ID and a flag indicating whether it
 * refers to the flight proper or the towflight. The flight reference is
 * considered valid if its database ID is valid.
 *
 * While it does not make sense, it is possible to construct a flight reference
 * referring to the towflight of a non-airtow flight. It is up to the user to
 * avoid this case.
 */
class FlightReference
{
	public:
		FlightReference ();
		FlightReference (dbId id, bool towflight);
		explicit FlightReference (const Flight &flight);
		virtual ~FlightReference ();

		static FlightReference flight (dbId id);
		static FlightReference towflight (dbId id);

		static const FlightReference invalid;

		dbId id () const;
		bool towflight () const;

		bool isValid () const;

		QString toString () const;
		QString toString (const QString &flightText, const QString &towflightText) const;
		QString toString (const QString &templateText, const QString &flightText, const QString &towflightText) const;

		bool operator== (const FlightReference &other) const;

	private:
		dbId _id;
		bool _towflight;
};

uint qHash (const FlightReference &flightReference);

#endif
