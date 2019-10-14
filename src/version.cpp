#include "version.h"

#include "text.h"
#include "i18n/notr.h"

/**
 * Preprocessor define APPLICATION_VERSION is set by the build system qmake
 *
 */
QString getVersion ()
{
	QString version = APPLICATION_VERSION;

	return version;
}
