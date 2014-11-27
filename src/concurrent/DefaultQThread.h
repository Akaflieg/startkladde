#ifndef DEFAULTQTHREAD_H_
#define DEFAULTQTHREAD_H_

#include <QThread>

class DefaultQThread: public QThread
{
	public:
		DefaultQThread (QObject *parent=NULL);
		virtual ~DefaultQThread ();

		static void sleep  (unsigned long secs ) { QThread:: sleep (secs ); }
		static void msleep (unsigned long msecs) { QThread::msleep (msecs); }
		static void usleep (unsigned long usecs) { QThread::usleep (usecs); }

	protected:
		virtual void run ();
};

#endif
