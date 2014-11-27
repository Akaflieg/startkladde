#include "signal.h"

#include <QTimer>

void invokeSlot (QObject *receiver, const char *slot)
{
	QTimer::singleShot (0, receiver, slot);
}
