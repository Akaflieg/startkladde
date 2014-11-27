#include "src/util/fileSystem.h"

#include <QString>
#include <QFileInfo>

/**
 * Returns the parent directory of the specified filename, if that exists, or
 * the specified default directory otherwise.
 */
QDir existingParentDirectory (const QString &filename, const QDir &defaultDirectory)
{
	// Ideally, if the parent directory does not exist, this function should
	// recursively try that directory's parent, until an existing directory is
	// found or the root directory (or one of the root directories on Windows)
	// is reached. Note, however, that QDir::cdUp does not work if the parent
	// directory does not exist, and QDir::isRoot does not seem to be true for
	// all root directories.
	// Note QFileInfo (...).dir ().path ()
	QDir dir=QFileInfo (filename).absoluteDir ();

	if (dir.exists ())
		return dir;
	else
		return defaultDirectory;
}
