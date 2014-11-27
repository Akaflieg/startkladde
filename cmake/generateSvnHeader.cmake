# This only detects subversion working copies if they contain a .svn directory.
# This seems to be the case for both svn and TortoiseSVN at the moment, but we
# may want to use the subversion client - if found - to determine whether the
# source directory is a working directory.

SET (svn_dir "${SOURCE_DIR}/.svn")

IF(EXISTS "${svn_dir}" AND IS_DIRECTORY "${svn_dir}")
	SET (SVN_IS_WC "true")
	
	# Looks like this is an SVN directory. Try to find SVN.
	FIND_PACKAGE (Subversion)
	
	if (SUBVERSION_FOUND)
		# Extract working copy information into SVN_... variables
		Subversion_WC_INFO (${SOURCE_DIR} SVN)

		MESSAGE (STATUS "SVN working copy, revision is ${SVN_REVISION}")

		SET (SVN_REVISION_KNOWN "true"              )
		SET (SVN_REVISION       "${SVN_WC_REVISION}")
	else ()
		MESSAGE (STATUS "SVN working copy, unknown revision (svn not found)")

		SET (SVN_REVISION_KNOWN "false")
		SET (SVN_REVISION       ""     )
	endif ()
ELSE ()
	MESSAGE (STATUS "Not an SVN working copy")

	SET (SVN_IS_WC          "false")
	SET (SVN_REVISION_KNOWN "false")
	SET (SVN_REVISION       ""     )
ENDIF ()

# Write the header file contents to a temporary file
file (WRITE svnVersion.h.tmp
	"#define SVN_IS_WC            ${SVN_IS_WC}         \n"
	"#define SVN_REVISION_KNOWN   ${SVN_REVISION_KNOWN}\n"
	"#define SVN_REVISION       \"${SVN_REVISION}\"    \n"
)

# Copy the file to the final file (only if it changed)
execute_process (COMMAND ${CMAKE_COMMAND} -E copy_if_different svnVersion.h.tmp svnVersion.h)
