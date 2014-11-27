#include "SimpleOperationMonitor.h"

#include <iostream>

#include "src/util/qString.h"
#include "src/i18n/notr.h"

SimpleOperationMonitor::SimpleOperationMonitor ()
{
}

SimpleOperationMonitor::~SimpleOperationMonitor ()
{
}

void SimpleOperationMonitor::cancel ()
{
	// No operation
}

void SimpleOperationMonitor::setStatus (const QString &text)
{
	std::cout << notr ("Status: ") << text << std::endl;
}

void SimpleOperationMonitor::setProgress (int progress, int maxProgress)
{
	if (maxProgress>=0)
		std::cout << (qnotr ("Progress: %1/%2 (%3%)").arg (progress).arg (maxProgress).arg (100*progress/(float)maxProgress)) << std::endl;
	else
		std::cout << notr ("Progress: ") << progress << std::endl;
}

void SimpleOperationMonitor::setEnded ()
{
	std::cout << notr ("Process ended") << std::endl;

}
