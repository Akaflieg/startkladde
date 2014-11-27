#include <QTimer>

void setTimerRunning (QTimer *timer, bool running)
{
	if (!timer)
		return;

	if (running)
		timer->start ();
	else
		timer->stop ();
}
