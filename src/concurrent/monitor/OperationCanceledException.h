/*
 * OperationCanceledException.h
 *
 *  Created on: 07.03.2010
 *      Author: Martin Herrmann
 */

#ifndef OPERATIONCANCELEDEXCEPTION_H_
#define OPERATIONCANCELEDEXCEPTION_H_

#include "src/StorableException.h"

class OperationCanceledException: public StorableException
{
	public:
		OperationCanceledException ();
		virtual ~OperationCanceledException ();

		virtual OperationCanceledException *clone () const;
		virtual void rethrow () const;
};

#endif
