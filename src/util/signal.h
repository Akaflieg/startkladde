#ifndef SIGNAL_H_
#define SIGNAL_H_

class QObject;

void invokeSlot (QObject *receiver, const char *slot);

#endif
