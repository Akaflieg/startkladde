/*
 * SkProcess.cpp
 *
 *  Created on: 27.07.2010
 *      Author: martin
 */

/*
 * On process termination:
 *   - The process may finish spontaneously or because we terminated it.
 *   - We want to be able to terminate the process. Thus, we
 *     need to store a pointer to the QProcess as long as it is running
 *   - In the finished slot, the stored pointer may alredy have been
 *     overwritten (by restarting the process). In this case, we may not use it
 *     for deletion.
 */


#include "SkProcess.h"

#include <QProcess>

#include "src/util/io.h"

SkProcess::SkProcess (QObject *parent):
	QObject (parent),
	process (NULL)
{
}

SkProcess::~SkProcess ()
{
	stop ();
}

void SkProcess::start (const QString &command)
{
    start(command, QStringList());
}

void SkProcess::start(const QString &command, const QStringList &arguments) {
    stop ();

    process=new QProcess (this);
    // process->setWorkingDirectory (...);

    connect (process, SIGNAL (readyReadStandardOutput ()), this, SLOT (outputAvailable ()));
    connect (process, SIGNAL (readyReadStandardError ()), this, SLOT (errorOutputAvailable ()));
    connect (process, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (processFinished (int, QProcess::ExitStatus)));

    /* eggert: start method without argument is not available in Qt 
    if (arguments.isEmpty())
        process->start(command, QIODevice::ReadOnly);
    else
        process->start (command, arguments, QIODevice::ReadOnly);
    */
    process->start (command, arguments, QIODevice::ReadOnly);

    process->closeWriteChannel ();
}

bool SkProcess::startAndWait (const QString &command)
{
	start (command);
	return process->waitForStarted ();
}

void SkProcess::stop ()
{
	if (process)
	{
		if (process->state ()==QProcess::NotRunning)
		{
			// The process is not running. We can delete it right away.
			delete process;
		}
		else
		{
			// The process may still be running. Terminate it, it will be
			// delete after it finishes.
			process->terminate ();
		}

		process=NULL;
	}
}

void SkProcess::outputAvailable ()
{
	// Might happen if the process finished or was restarted in the meantime
	if (!process) return;

	QString line;
	while (line=readLineUtf8 (*process).trimmed (), !line.isEmpty ())
		emit lineReceived (line);
}


void SkProcess::errorOutputAvailable ()
{
    // Might happen if the process finished or was restarted in the meantime
    if (!process) return;

    QString line = process->readAllStandardError();
    emit errorLineReceived(line);
}


void SkProcess::processFinished (int exitCode, QProcess::ExitStatus exitStatus)
{
	// The stored process pointer may already have been overwritten, for
	// example, if the process has been restarted and this slot is called for
	// the "old" QProcess instance. Thus, we cannot use the stored pointer for
	// accessing the process and have to use the sender() instead.
	QProcess *p=dynamic_cast<QProcess *> (sender ());

	// In any case, whether the process was terminated or finished
	// spontaneously, we delete the QProcess instance. It may not be wise to
	// delete the sender of the signal that invoked a slot, so we defer
	// deletion to the event loop.
	p->deleteLater ();

	if (process==p)
	{
		// We have a pointer to the process stored. This means that probably
		// the process finished spontaneously.
		emit exited (exitCode, exitStatus);

		// Set the pointer to NULL so it is not accessed later.
		// If the stored pointer points to something other than the finished
		// process, it is not touched.
		process=NULL;
	}
}


void SkProcess::splitCommand (QString &commandProper, QString &parameters, const QString &commandWithParameters)
{
	int firstSpace=commandWithParameters.indexOf (' ');

	if (firstSpace<0)
	{
		// No space
		commandProper=commandWithParameters;
		parameters.clear ();
	}
	else
	{
		// foo bar
		// 0123456
		commandProper=commandWithParameters.left (firstSpace);
		parameters=commandWithParameters.mid (firstSpace+1);
	}
}
