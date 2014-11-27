#include "bool.h"

bool stringToBool (const QString &text)
{
	if (text.toInt ()==0)
		return false;
	else
		return true;
}
