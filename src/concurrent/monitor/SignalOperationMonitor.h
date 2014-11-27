/*
 * SignalOperationMonitor.h
 *
 *  Created on: 03.03.2010
 *      Author: Martin Herrmann
 */

#ifndef SIGNALOPERATIONMONITOR_H_
#define SIGNALOPERATIONMONITOR_H_

#include <QObject>

#include "OperationMonitor.h"

class SignalOperationMonitor: public QObject, public OperationMonitor
{
	Q_OBJECT

	public:
		SignalOperationMonitor ();
		virtual ~SignalOperationMonitor ();

		bool getEnded () const;
		const QString &getStatus () const;
		int getProgress () const { return progress; }
		int getMaxProgress () const { return maxProgress; }

	public slots:
		virtual void cancel ();

	signals:
		void canceled ();

		void statusChanged (QString text);
		void progressChanged (int progress, int maxProgress);
		void ended ();

	private:
		bool hasEnded;
		QString status;
		int progress, maxProgress;

		virtual void setStatus (const QString &text);
		virtual void setProgress (int progress, int maxProgress);
		virtual void setEnded ();
};

#endif
