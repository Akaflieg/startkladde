#include "synchronized.h"

Synchronizer::Synchronizer (QRecursiveMutex *mutex):
	QMutexLocker (mutex), done (false)
{
}

Synchronizer::~Synchronizer ()
{
}
