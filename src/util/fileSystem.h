#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <QDir>

class QString;

QDir existingParentDirectory (const QString &filename, const QDir &defaultDirectory);

#endif
