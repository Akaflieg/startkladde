#ifndef SKMOVIE_H_
#define SKMOVIE_H_

#include <QObject>
#include <QSharedPointer>
#include <QTemporaryFile>

class QMovie;
class QByteArray;
class QIODevice;

/**
 * A container for a QMovie that stores data in a temporary file and can be
 * copied
 *
 * The data source used to initialize the SkMovie may become unavailable at any
 * time after the SkMovie has been initialized.
 *
 * Copying of an SkMovie is fast. The underlying data is deleted when the last
 * copy of the SkMovie is destroyed.
 */
class SkMovie
{
	public:
		SkMovie ();
		SkMovie (QIODevice *device, const QByteArray &format=QByteArray ());
		SkMovie (const QString &fileName, const QByteArray &format=QByteArray ());

		virtual ~SkMovie ();

		// Not a QSharedPointer because it may not be used after the SkMovie
		// has been deleted.
		QMovie *getMovie () { return movie.data (); }

	private:
		QSharedPointer<QTemporaryFile> tempFile;
		QSharedPointer<QMovie> movie;
};

#endif
