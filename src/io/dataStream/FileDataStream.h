#ifndef FILEDATASTREAM_H_
#define FILEDATASTREAM_H_

#include <stdint.h>

#include "src/io/dataStream/DataStream.h"

class QFile;
class QMutex;
class QTimer;

/**
 * A DataStream implementation that reads data from a file, one line at a time,
 * with a constant, configurable delay between lines.
 *
 * The file name and delay are configured using the setFileName and setDelay
 * methods. After that, the stream can be opened.
 *
 * This DataStream implementation will block on opening until the file has been
 * opened, which should typically be instantaneous; however, if the file is on
 * a slow medium (such as a network path), this may take some time.
 *
 * This class is thread safe.
 */
class FileDataStream: public DataStream
{
	Q_OBJECT

	public:
		FileDataStream (QObject *parent);
		virtual ~FileDataStream ();

		void setFileName (const QString &fileName);
		void setDelay (int milliseconds);

	protected:
		// DataStream methods
		virtual void openStream ();
		virtual void closeStream ();
		virtual bool streamParametersCurrent ();

	private:
		// There are two different mutexes: one for protecting the parameters
		// and one for protecting the back-end. This is so that a blocking
		// back-end operation does not block other operations.
	    QMutex *_parameterMutex;
	    QMutex *_backEndMutex;

	    // Parameters
		QString _fileName;
		int _delayMs;

		// Back-end
	    QFile *_file;
	    QTimer *_timer;

	private slots:
		void timerSlot ();
};

#endif
