/*
 * SimpleOperationMonitor.h
 *
 *  Created on: Aug 2, 2009
 *      Author: Martin Herrmann
 */

#ifndef SIMPLEOPERATIONMONITOR_H_
#define SIMPLEOPERATIONMONITOR_H_

#include "OperationMonitor.h"

class SimpleOperationMonitor: public OperationMonitor
{
	public:
		SimpleOperationMonitor ();
		virtual ~SimpleOperationMonitor ();

		virtual void cancel ();

	private:
		// ** Operation feedback
		virtual void setStatus (const QString &text);
		virtual void setProgress (int progress, int maxProgress);
		virtual void setEnded ();
};

#endif
