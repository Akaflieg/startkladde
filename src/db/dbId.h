#ifndef DBID_H_
#define DBID_H_

/**
 * This is not part of Database because a lot of classes (e. g. all Entity
 * subclasses) depend on this type, but don't necessarily depend on Database.
 * If this were defined in Database.h, almost everything would have to be
 * rebuilt when Database is changed.
 */

#include <QtGlobal>

typedef quint32 dbId;

static const dbId invalidId=0;

bool idInvalid (dbId id);
bool idValid (dbId id);

#endif
