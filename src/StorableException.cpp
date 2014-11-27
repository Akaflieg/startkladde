#include "StorableException.h"

// We need to define this, and define it non-inline, or the compiler won't know
// where to put the vtable.
StorableException::~StorableException ()
{
}
