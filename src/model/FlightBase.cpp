#include "FlightBase.h"


FlightBase::FlightBase ()
{
	initialize (invalidId);
}

FlightBase::FlightBase (dbId id)
{
	initialize (id);
}

FlightBase::~FlightBase ()
{
}

void FlightBase::initialize (dbId id)
{
	this->id=id;

	planeId         =invalidId;
	numLandings     =invalidId;
	pilotId         =invalidId;
	copilotId       =invalidId;
	towpilotId      =invalidId;
	launchMethodId  =invalidId;
	towplaneId      =invalidId;

	type          =typeNone;
	mode          =modeLocal;
	towflightMode =modeLocal;

	departed        =false;
	landed          =false;
	towflightLanded =false;

    vfId =invalidId;
}
