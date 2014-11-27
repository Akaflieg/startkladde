#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include <QString>
#include <QStringList>

#include "src/i18n/notr.h"

QString getEnvironmentVariable (const QString& name);
QStringList getSystemPath (const QString &environmentVariable=notr ("PATH"));

#endif
