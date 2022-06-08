#include "src/io/dataStream/FileDataStream.h"

#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>

#include "src/concurrent/DefaultQThread.h"

/*
 * Note: this class is thread safe. Keep it that way.
 */

// ******************
// ** Construction **
// ******************

FileDataStream::FileDataStream (QObject *parent): DataStream (parent),
	_parameterMutex (new QMutex ()),
	_backEndMutex   (new QMutex ()),
	_delayMs (0)
{
	// Create the file and the timer. _file and _timer will be deleted
	// automatically by their parent (this).
	_file=new QFile (this);
	_timer=new QTimer (this);

	connect (_timer, SIGNAL (timeout ()), this, SLOT (timerSlot ()));
}

FileDataStream::~FileDataStream ()
{
	delete _parameterMutex;
	delete _backEndMutex;
}


// ****************
// ** Parameters **
// ****************

/**
 * Sets the name of the file to use when the stream is opened the next time.
 *
 * This method is thread safe.
 */
void FileDataStream::setFileName (const QString &fileName)
{
	QMutexLocker parameterLocker (_parameterMutex);

	_fileName=fileName;
}

/**
 * Sets the line delay to use when the stream is opened the next time.
 *
 * This method is thread safe.
 */
void FileDataStream::setDelay (int milliseconds)
{
	QMutexLocker parameterLocker (_parameterMutex);

	_delayMs=milliseconds;
}


// ************************
// ** DataStream methods **
// ************************

/**
 * Implementation of DataStream::openStream ().
 *
 * This method is thread safe.
 */
void FileDataStream::openStream ()
{
	// Lock the parameter mutex and make a copy of the parameters. Unlock the
	// parameter mutex afterwards.
	QMutexLocker parameterLocker (_parameterMutex);
	QString fileName=_fileName;
	int delayMs=_delayMs;
	parameterLocker.unlock ();

	// Lock the back-end mutex and open the file. Unlock the mutex before
	// calling the base class. It will also be unlocked when the method returns.
	QMutexLocker backEndLocker (_backEndMutex);

	// A delay to simulate a blocking open method. This can be used to test a
	// background implementation.
	//qDebug () << "FileDataStream: pre-open delay";
	//DefaultQThread::sleep (2);

	// If the file is currently open, close it.
	if (_file->isOpen ())
		_file->close ();

	// Open the file for reading in text mode
	_file->setFileName (fileName);
	bool success=_file->open (QFile::ReadOnly | QFile::Text);

	if (success)
	{
		// The file was successfully opened. Start the read timer with the
		// specified delay.
		_timer->setInterval (delayMs);
		_timer->start ();
		backEndLocker.unlock ();
		streamOpened ();
	}
	else
	{
		// The file could not be opened.
		backEndLocker.unlock ();
		streamError (_file->errorString ());
	}
}

/**
 * Implementation of DataStream::closeStream ().
 *
 * This method is thread safe.
 */
void FileDataStream::closeStream ()
{
	// Lock the back-end mutex, close the file and stop the timer. Note that
	// there may still be a timer event in the queue.
	QMutexLocker backEndLocker (_backEndMutex);

	_timer->stop ();
	_file->close ();
}

/**
 * Implementation of DataStream::streamParametersCurrent ().
 *
 * This method is thread safe.
 */
bool FileDataStream::streamParametersCurrent ()
{
	QMutexLocker backEndLocker (_backEndMutex);
	if (_file->isOpen ())
		return false;
	QString activeFileName=_file ->fileName ();
	int     activeDelayMs =_timer->interval ();
	backEndLocker.unlock ();

	QMutexLocker parameterLocker (_parameterMutex);
	QString configuredFileName=_fileName;
	int     configuredDelayMs =_delayMs;
	parameterLocker.unlock ();

	return
		configuredFileName == activeFileName &&
		configuredDelayMs  == activeDelayMs;
}


// ***********
// ** Timer **
// ***********

/**
 * Called at regular intervals to read a line from the file.
 *
 * This method is thread safe.
 */
void FileDataStream::timerSlot ()
{
	// Lock the back-end mutex, read a line of data and close the file if EOF
	// has been reached. Unlock the mutex before calling the base class.
	QMutexLocker backEndLocker (_backEndMutex);

	// The file may have been closed after a timer event was queued. We
	// therefore have to verify that the file is still open.
	if (_file->isOpen ())
	{
		// Read a single line from the file. On EOF, this will return an empty
		// string.
		QByteArray line=_file->readLine ();

		if (line.isEmpty ())
		{
			// We reached the end of the file. Stop the timer and close the
			// file. We currently report this as an error; we could as well
			// silently rewind the file.
			//qDebug () << "FileDataStream: EOF";
			_timer->stop ();
			_file->close ();

			backEndLocker.unlock ();
			streamError (tr ("End of file reached"));
		}
		else
		{
			//qDebug () << "FileDataStream:" << line.trimmed ();
			// Emit a signal with the received data.
			backEndLocker.unlock ();
			emit dataReceived (line);
		}
	}
}
