#include "synchronized.h"

Synchronizer::Synchronizer (QMutex *mutex):
	QMutexLocker (mutex), done (false)
{
}

Synchronizer::~Synchronizer ()
{
}
