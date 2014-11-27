#include "src/FlightReference.h"

#include "src/model/Flight.h"

/**
 * An invalid flight reference. Can be invoked by
 *     FlightReference::invalid
 *
 * This is more expressive than FlightReference () and more terse than
 * FlightReference (invalidId, false)
 */
const FlightReference FlightReference::invalid;


/**
 * Constructs an invalid flight reference
 */
FlightReference::FlightReference ():
	_id (invalidId), _towflight (false)
{
}

/**
 * Constructs a flight reference with the given ID, referring to the towflight
 * if towflight is true or the flight proper if it is false
 */
FlightReference::FlightReference (dbId id, bool towflight):
	_id (id), _towflight (towflight)
{
}

/**
 * Constructs a flight reference to the given flight or towflight
 *
 * If the flight is a towflight, a reference to the towflight with the flight's
 * ID is created. Otherwise, a reference to the flight with the flight's ID is
 * created.
 *
 * This depends on the flight's flight type being set to airtow if it is an
 * airtow.
 */
FlightReference::FlightReference (const Flight &flight):
	_id (flight.getId ()), _towflight (flight.isTowflight ())
{
}

/**
 * Creates and returns a flight reference to the flight proper with the given
 * ID.
 */
FlightReference FlightReference::flight (dbId id)
{
	return FlightReference (id, false);
}

/**
 * Creates and returns a flight reference to the towflight with the given ID.
 */
FlightReference FlightReference::towflight (dbId id)
{
	return FlightReference (id, true);
}

FlightReference::~FlightReference ()
{
}

/**
 * Returns the ID this flight reference refers to
 */
dbId FlightReference::id () const
{
	return _id;
}

/**
 * Returns true if this flight reference refers to a towflight, or false if not
 */
bool FlightReference::towflight () const
{
	return _towflight;
}

/**
 * Returns true if the flight ID is valid
 *
 * This does not check whether the flight actually exists in the database.
 */
bool FlightReference::isValid () const
{
	return idValid (_id);
}

/**
 * Compares two flight references for equality
 *
 * Two flight references are considered equal if both their ID and towflight
 * flag are equal.
 */
bool FlightReference::operator== (const FlightReference &other) const
{
	return (other._id==this->_id) && (other._towflight==this->_towflight);
}

/**
 * A hash function required for using FlightReference as key type in a QHash
 *
 * @see QHash
 */
uint qHash (const FlightReference &flightReference)
{
	if (flightReference.towflight ())
		return 2*flightReference.id ()+1;
	else
		return 2*flightReference.id ();
}

/**
 * Creates a string describing the flight reference
 *
 * The template text must contain "%1" and "%2". "%1" will be replaced with the
 * flight text or the towflight text, depending on the towflight flag. "%2" will
 * be replaced with the flight ID.
 */
QString FlightReference::toString (const QString &templateText, const QString &flightText, const QString &towflightText) const
{
	if (_towflight)
		return templateText.arg (towflightText).arg (_id);
	else
		return templateText.arg (flightText).arg (_id);
}

/**
 * Creates a string describing the flight reference with a default template text
 *
 * The template text is "%1 %2", giving a string like "flight 1" or "towflight
 * 1".
 *
 * @see toString (const QString &, const QString &, const QString &)
 */
QString FlightReference::toString (const QString &flightText, const QString &towflightText) const
{
	return toString ("%1 %2", flightText, towflightText);
}
