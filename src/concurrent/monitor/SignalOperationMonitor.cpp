#include "SignalOperationMonitor.h"

SignalOperationMonitor::SignalOperationMonitor ():
	hasEnded (false), progress (0), maxProgress (0)
{
}

SignalOperationMonitor::~SignalOperationMonitor ()
{
}

void SignalOperationMonitor::cancel ()
{
	emit canceled ();
	OperationMonitor::cancel ();
}

void SignalOperationMonitor::setStatus (const QString &text)
{
	status=text;
	emit statusChanged (text);
}

void SignalOperationMonitor::setProgress (int progress, int maxProgress)
{
	this->progress=progress;
	this->maxProgress=maxProgress;
	emit progressChanged (progress, maxProgress);
}

void SignalOperationMonitor::setEnded ()
{
	hasEnded=true;
	emit ended ();
}

bool SignalOperationMonitor::getEnded () const
{
	return hasEnded;
}

const QString &SignalOperationMonitor::getStatus () const
{
	return status;
}
