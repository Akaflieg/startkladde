#include "synchronized.h"

Synchronizer::Synchronizer (QMutex *mutex):
	QMutexLocker (mutex), done (false)
{
}

Synchronizer::Synchronizer (QRecursiveMutex *mutex):
    QMutexLocker (mutex), done (false)
{
}

Synchronizer::~Synchronizer ()
{
}
