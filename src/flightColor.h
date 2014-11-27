#ifndef COLOR_H_
#define COLOR_H_

#include <QColor>

#include "src/model/Flight.h"

// TODO move to Flight
QColor flightColor (Flight::Mode, bool, bool, bool, bool);

#endif

