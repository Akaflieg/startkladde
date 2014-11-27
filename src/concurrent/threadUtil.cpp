#include "threadUtil.h"

#include <QApplication>
#include <QThread>

QThread *guiThread ()
{
	return qApp->thread ();
}

bool isGuiThread ()
{
	return QThread::currentThread ()==guiThread ();
}
