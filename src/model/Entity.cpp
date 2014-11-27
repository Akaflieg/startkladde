#include "Entity.h"

// ******************
// ** Construction **
// ******************

Entity::Entity ()
{
	id=0;
}

Entity::Entity (dbId id)
{
	this->id=id;
}

Entity::~Entity ()
{
}
