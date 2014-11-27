#include "OperationCanceledException.h"

OperationCanceledException::OperationCanceledException ()
{
}

OperationCanceledException::~OperationCanceledException ()
{
}

OperationCanceledException *OperationCanceledException::clone () const
{
	return new OperationCanceledException ();
}

void OperationCanceledException::rethrow () const
{
	throw OperationCanceledException ();
}
