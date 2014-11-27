#include "dbId.h"

bool idValid (dbId id)
{
	return !idInvalid (id);
}

bool idInvalid (dbId id)
{
	return (id==0);
}
