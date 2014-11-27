#include "DbEventMonitor.h"

DbEventMonitor::DbEventMonitor (QObject &source, const char *signal, DbEventMonitor::Listener &listener):
	listener (listener)
{
	connect (&source, signal, this, SLOT (dbEvent (DbEvent)));
}

DbEventMonitor::~DbEventMonitor ()
{
}

void DbEventMonitor::dbEvent (DbEvent event)
{
	listener.dbEvent (event);
}
