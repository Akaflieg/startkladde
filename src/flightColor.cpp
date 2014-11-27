#include "src/flightColor.h"

// Conditions. These macros expand to an expression.
#define isTrue(  x) ((x)==true)
#define isFalse( x) ((x)==false)
#define dontCare(x) (true)
#define isLocal(  x) ((x)==Flight::modeLocal  )
#define isComing( x) ((x)==Flight::modeComing )
#define isLeaving(x) ((x)==Flight::modeLeaving)

// Indirectors. These macros expand to names of macros taking one argument.
#define t isTrue
#define f isFalse
#define x dontCare
#define local   isLocal
#define coming  isComing
#define leaving isLeaving

// Generates a condition
#define farbe(modeCondition, towflightCondition, departedCondition, landedCondition, errorCondition, resultingColor) \
	if ( \
		modeCondition     (mode     ) && \
		towflightCondition(towflight) && \
		departedCondition (departed ) && \
		landedCondition   (landed   ) && \
		errorCondition    (error    ))   \
		return (resultingColor);

// Colors
#define red         QColor (255,  0,  0)
#define lightRed    QColor (255,127,127)
#define blue        QColor (127,127,255)
#define lightBlue   QColor (191,191,255)
#define towBlue     QColor (162,162,216)
#define green       QColor (  0,255,  0)
#define lightGreen  QColor (191,255,191)
#define yellow      QColor (255,255,  0)
#define lightYellow QColor (255,255,191)
#define magenta     QColor (255,255,  0)
#define white       QColor (255,255,255)


// TODO move to flight
/**
 * Determines the color to use for a given flight.
 *
 * @param mode the mode of the flight
 * @param error whether the flight is erroneous
 * @param towflight whether the flight is a towflight
 * @param departed whether the flight has departed
 * @param landed whether the flight has landed
 * @return the color for the flight as QColor
 */
QColor flightColor (Flight::Mode mode, bool error, bool towflight, bool departed, bool landed)
{
	// The purpose of this contraption is to be able to verify that each case
	// has been handled.
	// The columns contain the respective indirector macros - for the mode
	// column, one of dontCare, local, coming or leaving; for the boolean
	// columns one of t (the variable must be true), f (the variable must be
	// false) or x (the variable is ignored). The first matching condition is
	// used.

	//     Mode      Towflight
	//                  Departed
	//                     Landed
	//                        Erroneous
	//                           Color
	//
	// Erroneous flights
	farbe (dontCare, t, x, x, t, lightRed); // Erroneous towflight
	farbe (dontCare, f, x, x, t, lightRed); // Erroneous flight

	// Program errors - if a flight has landed, but not departed, the error
	// flag must be true
	farbe (leaving , x, f, t, x, magenta ); // Leaving flight
	farbe (local   , x, f, t, x, magenta ); // Local flight

	// Coming flights
	farbe (coming, f, x, f, f, lightBlue ); // Flying
	farbe (coming, f, x, t, f, lightGreen); // Landed
	farbe (coming, t, x, x, f, magenta   ); // Program error - coming towflights are not valid

	// Leaving flights
	farbe (leaving, f, f, x, f, lightYellow); // Prepared
	farbe (leaving, f, t, x, f, lightGreen ); // Leaving (landing is not recorded)
	farbe (leaving, t, f, x, f, yellow     ); // Prepared towflight (cannot happen)
	farbe (leaving, t, t, x, f, lightGreen ); // Leaving towflight

	// Local flights
	farbe (local, f, f, x, f, lightYellow); // nicht gestartet
	farbe (local, f, t, f, f, lightBlue  ); // Flug in der Luft
	farbe (local, f, t, t, f, lightGreen ); // gelandeter Flug
	farbe (local, t, f, x, f, yellow     ); // nicht gestarteter Schleppflug (gibt es nicht)
	farbe (local, t, t, f, f, towBlue    ); // Schleppflug in der Luft
	farbe (local, t, t, t, f, lightGreen ); // gelandeter Schleppflug

	return QColor (255, 255, 255);	// Default: weiß
}
