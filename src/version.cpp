#include "version.h"

#include "text.h"
#include "i18n/notr.h"

// Generated headers
#include "build.h"
#include "svnVersion.h"

/**
 * Returns a descriptive version string intended for display to the user
 *
 * The string contains the version, which is similar to:
 *   - 2.1.3       if not built from an SVN working copy
 *   - 2.1.3-r1234 if built from an SVN working copy with known revision
 *   - 2.1.3-svn   if built from an SVN working copy with unknown revision
 *
 * Also included in the version string is the build type
 */
QString getVersion ()
{
	QString version=qnotr ("Version %1.%2.%3")
		.arg (BUILD_VERSION_MAJOR)
		.arg (BUILD_VERSION_MINOR)
		.arg (BUILD_VERSION_REVISION);

	if (SVN_IS_WC)
	{
		// The program is being built from an SVN working copy. We may or may
		// not know the SVN revision, depending on whether the SVN client was
		// found.
		if (SVN_REVISION_KNOWN)
			version+=qnotr ("-r%1").arg (SVN_REVISION);
		else
			version+=qnotr ("-svn");
	}

	QString buildType (BUILD_TYPE);
	if (!buildType.isEmpty ())
		version+=qnotr (" (build type: %1)").arg (firstToLower (buildType));

	return version;
}
