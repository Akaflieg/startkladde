#ifndef EVENTTIMETRACKER_H_
#define EVENTTIMETRACKER_H_

#include <QHash>
#include <QString>
#include <QTime>

class QDateTime;

class EventTimeTracker
{
	public:
		EventTimeTracker ();
		virtual ~EventTimeTracker ();

		void eventNow (const QString &value);
		QTime timeSinceEvent (const QString &value);
		bool eventWithin (const QString &value, const QTime &span);

	private:
		QHash<QString, QDateTime> _lastEvent;
};

#endif
