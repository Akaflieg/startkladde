#include "DefaultQThread.h"

DefaultQThread::DefaultQThread (QObject *parent):
	QThread (parent)
{
}

DefaultQThread::~DefaultQThread ()
{
}

void DefaultQThread::run ()
{
	exec ();
}
