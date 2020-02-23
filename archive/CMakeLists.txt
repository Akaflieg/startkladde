##############
## Settings ##
##############

project("startkladde")

set (startkladde_VERSION_MAJOR 2)
set (startkladde_VERSION_MINOR 1)
set (startkladde_VERSION_REVISION 1)
set (full_version ${startkladde_VERSION_MAJOR}.${startkladde_VERSION_MINOR}.${startkladde_VERSION_REVISION})

cmake_minimum_required( VERSION 2.8.11 ) #2.8.11 for Qt5

#################
## cmake setup ##
#################

# If the user specifies -DCMAKE_BUILD_TYPE on the command line, take their
# definition and dump it in the cache along with proper documentation, otherwise
# set CMAKE_BUILD_TYPE to Release prior to calling PROJECT()
# This must be performed before PROJECT().
IF(DEFINED CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ELSE()
	SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ENDIF()

# Include additional cmake scripts (e. g. finding the MySQL library)
SET (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")

##############
## Packages ##
##############

# For MinGW, we need to copy some libraries from the MinGW binary directory
# later. Determine the binary directory from the C compiler location.
if (MINGW)
	get_filename_component (MINGW_BINARY_DIR ${CMAKE_C_COMPILER} PATH)
endif ()

SET (MYSQL_DIR "" CACHE PATH "Base directory of the MySQL installation (the directory containing bin/, lib/ and include/, only required if MySQL is not autodetected (typically on Windows)")

# Hack: prevent an error message if not found
SET (RUBY_LIBRARY            "not_required")
SET (RUBY_INCLUDE_DIR        "not_required")
SET (RUBY_CONFIG_INCLUDE_DIR "not_required")

# Find packages
FIND_PACKAGE (Qt5Widgets        REQUIRED)
FIND_PACKAGE (Qt5Sql            REQUIRED)
FIND_PACKAGE (Qt5Xml            REQUIRED)
FIND_PACKAGE (Qt5Network        REQUIRED)
FIND_PACKAGE (Qt5LinguistTools  REQUIRED)
FIND_PACKAGE (MySQL 5.0    REQUIRED)
FIND_PACKAGE (Acpi                 )
FIND_PACKAGE (Ruby         REQUIRED)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


# For gcc and release mode, enable some options that make the binary smaller
if (CMAKE_COMPILER_IS_GNUCXX)
	set (CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}        -DQT_NO_DEBUG") # Disable QT debug code
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s"           ) # Strip binary
endif ()


######################
## Compile settings ##
######################

# TODO: only for the GNU compiler, CMAKE_COMPILER_IS_GNUCXX (and test on MinGW
# and Linux)
# Qt5Widgets_EXECUTABLE_COMPILE_FLAGS contains a special flag needed for qt5 (should get obsolete after a time...)
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

# Dump variables
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()


##################
## Source files ##
##################

# Make lists of source files
file (GLOB_RECURSE startkladde_SOURCES   RELATIVE ${PROJECT_SOURCE_DIR} src/*.cpp   )
file (GLOB_RECURSE startkladde_FORMS     RELATIVE ${PROJECT_SOURCE_DIR} src/*.ui    )
file (GLOB_RECURSE startkladde_RESOURCES RELATIVE ${PROJECT_SOURCE_DIR} config/*.qrc)

# Some files cannot be MOCed automatically because the header file name does not
# match the class name (?). MOC them manually.
SET (startkladde_MOC
	src/gui/widgets/AcpiWidget.h
	src/model/objectList/ObjectListModel.h
)

if (${ACPI_FOUND})
	SET (AcpiWidget libacpi)
	SET (libs ${libs} ${ACPI_LIBRARIES})
else ()
	SET (AcpiWidget dummy)
endif ()

# Serial
# FIXME! release library
#SET (libs ${libs} ${QT_LIBRARY_DIR}/libSerialPortd0.a)

# Tweak source lists
LIST (REMOVE_ITEM startkladde_SOURCES src/gui/widgets/AcpiWidget_libacpi.cpp)
LIST (REMOVE_ITEM startkladde_SOURCES src/gui/widgets/AcpiWidget_dummy.cpp)
LIST (APPEND      startkladde_SOURCES src/gui/widgets/AcpiWidget_${AcpiWidget}.cpp)
LIST (APPEND      startkladde_SOURCES "${PROJECT_BINARY_DIR}/CurrentSchema.cpp")

# We found the Ruby package, so hopefully, we have erb
ADD_CUSTOM_COMMAND (
	OUTPUT "${PROJECT_BINARY_DIR}/CurrentSchema.cpp"
	MAIN_DEPENDENCY src/db/schema/CurrentSchema.cpp.erb
	DEPENDS src/db/migrations/current_schema.yaml
	WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
	COMMAND erb -T 1 src/db/schema/CurrentSchema.cpp.erb >${PROJECT_BINARY_DIR}/CurrentSchema.cpp
)

# Handle Qt sources
QT5_WRAP_CPP      (startkladde_HEADERS_MOC   ${startkladde_MOC}      )
QT5_WRAP_UI       (startkladde_FORMS_HEADERS ${startkladde_FORMS}    )
QT5_ADD_RESOURCES (startkladde_RESOURCES_RCC ${startkladde_RESOURCES})

add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5Sql_DEFINITIONS})
add_definitions(${Qt5Xml_DEFINITIONS})
add_definitions(${Qt5Network_DEFINITIONS})

include_directories (
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
    ${MYSQL_INCLUDE_DIR}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Sql_INCLUDE_DIRS}
    ${Qt5Xml_INCLUDE_DIRS}
)


# This must be implemented as a macro, otherwise INSTALL_LIBS will be local
macro (copy_library source_directory library_name)
	message (STATUS "Copying library ${library_name} from ${source_directory}")

	# Copy the library to the build directory
	# TODO do this on/after build, not every time we run cmake.
	file (COPY ${source_directory}/${library_name} DESTINATION ${PROJECT_BINARY_DIR})

	# Add the library to the list of libraries to install
	set (INSTALL_LIBS ${INSTALL_LIBS} ${library_name})
endmacro ()

if (WIN32 AND MINGW)
	copy_library (${MINGW_BINARY_DIR} mingwm10.dll)
	copy_library (${MINGW_BINARY_DIR} libgcc_s_dw2-1.dll)
endif ()


# TODO: Qt and MySQL are required, so this check should not be necessary
if (WIN32 AND QT_BINARY_DIR AND MYSQL_INCLUDE_DIR)
	copy_library (${QT_BINARY_DIR} QtCore4.dll)
	copy_library (${QT_BINARY_DIR} QtGui4.dll)
	copy_library (${QT_BINARY_DIR} QtSql4.dll)
	copy_library (${QT_BINARY_DIR} QtXml4.dll)
	copy_library (${QT_BINARY_DIR} QtNetwork4.dll)

	copy_library (${MYSQL_LIBRARY_DIR} libmysql.dll)

#	# TODO should not be hardcoded
#    IF(CMAKE_BUILD_TYPE MATCHES "Debug")
#		message (STATUS "Copying library ${Qwt6_Qt4_DEBUG_LIBRARY}")
#		file (COPY ${Qwt6_Qt4_DEBUG_LIBRARY} DESTINATION ${PROJECT_BINARY_DIR})
#		set (INSTALL_LIBS ${INSTALL_LIBS} qwtd.dll)
#	ELSE ()
#		message (STATUS "Copying library ${Qwt6_Qt4_LIBRARY}")
#		file (COPY ${Qwt6_Qt4_LIBRARY} DESTINATION ${PROJECT_BINARY_DIR})
#		set (INSTALL_LIBS ${INSTALL_LIBS} qwt.dll)
#	ENDIF()

	file (COPY ${QT_PLUGINS_DIR}/sqldrivers/qsqlmysql4.dll DESTINATION ${PROJECT_BINARY_DIR}/plugins/sqldrivers/)
endif()

if (WIN32)
	SET (startkladde_WINDOWS_RESOURCES config/startkladde.rc)
endif ()

configure_file ("${PROJECT_SOURCE_DIR}/src/build.h.in" "${PROJECT_BINARY_DIR}/build.h")


##################
## Translations ##
##################

# Make a list of translation source files (absolute paths)
file (GLOB translation_sources translations/*.ts)

# Create the translations/ directory in the build directory
file (MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/translations)

# Set the OUTPUT_LOCATION of the translation source files to the translations/
# directory in the build directory. QT5_ADD_TRANSLATION will place the compiled
# translations there.
SET_SOURCE_FILES_PROPERTIES (${translation_sources} PROPERTIES OUTPUT_LOCATION ${PROJECT_BINARY_DIR}/translations)

# Add rules for compiling the translation files
# Note that QT5_CREATE_TRANSLATION will delete the translation sources (*.ts)
# on "make clean".
QT5_ADD_TRANSLATION(translation_files ${translation_sources})

ADD_CUSTOM_TARGET (lupdate
	COMMAND ${QT_LUPDATE_EXECUTABLE} src -ts ${translation_sources}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


################
## Executable ##
################

ADD_EXECUTABLE(startkladde
	WIN32 # GUI application on Windows - ignored on other platforms
	${startkladde_SOURCES} 
	${startkladde_MOC} 
    ${startkladde_FORMS_HEADERS} 
    ${startkladde_RESOURCES_RCC}
    ${startkladde_WINDOWS_RESOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/svnVersion.h
	${translation_files}
)

## Serial library
add_subdirectory( 3rdparty/qserialdevice/src lib )
target_link_libraries(startkladde qserialdevice)


# TODO: on Unix, we used to link against mysql explicitly (even though it is
# only used via the Qt plugin) to avoid "Error in my_thread_global_end(): 1
# threads didn't exit" in specific situations (e. g. exception on open).
# Is this still required?

TARGET_LINK_LIBRARIES (
    startkladde
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Sql_LIBRARIES}
    ${libs}
)


#########
## SVN ##
#########

# See http://stackoverflow.com/questions/3780667/use-cmake-to-get-build-time-svn-revision

ADD_CUSTOM_TARGET (svnheader ALL)

# Create svnVersion.h using cmake script
ADD_CUSTOM_COMMAND(
	TARGET svnheader
	COMMAND ${CMAKE_COMMAND}
		-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} 
		-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generateSvnHeader.cmake
)

# Mark svnVersion.h as generated
set_source_files_properties (
	${CMAKE_CURRENT_BINARY_DIR}/svnVersion.h
    PROPERTIES GENERATED TRUE
    HEADER_FILE_ONLY TRUE)

# The executable depends on the svnheader target
add_dependencies (startkladde svnheader)

# Linker flags
IF (WIN32)
	# Required for inheriting from QwtPlot; seems to be related to the fact that
	# QwtPlot is imported from a .dll.
	set_property (TARGET startkladde PROPERTY LINK_FLAGS "-Wl,--enable-auto-import")
ENDIF ()


#############
## Install ##
#############

#include (InstallRequiredSystemLibraries)

# Install path definitions
# On Windows, everything is installed into the application directory. On other
# systems, different files are installed into different system directories.
IF (WIN32)
	# Common
	SET (EXECUTABLE_INSTALL_DIR      ".")
	SET (TRANSLATIONS_INSTALL_DIR    ".")
	SET (PLUGINS_INSTALL_DIR         ".")
	# Windows specific
	SET (QT_PLUGINS_INSTALL_DIR      ".")
	SET (QT_TRANSLATIONS_INSTALL_DIR "translations")
ELSE ()
	# Common
	SET (EXECUTABLE_INSTALL_DIR   "bin"               )
	SET (TRANSLATIONS_INSTALL_DIR "share/startkladde" )
	SET (PLUGINS_INSTALL_DIR      "lib/startkladde"   )
	# Non-Windows specific
	SET (MENU_INSTALL_DIR         "share/applications")
	SET (ICON_INSTALL_DIR         "share/pixmaps"     )
ENDIF ()

# Install the executable, translations, external plugins
# For directories installed from the source directory, ignore the .svn
# subdirectory.
install (TARGETS   startkladde                        DESTINATION ${EXECUTABLE_INSTALL_DIR}                       )
install (DIRECTORY ${PROJECT_BINARY_DIR}/translations DESTINATION ${TRANSLATIONS_INSTALL_DIR}                     )
install (DIRECTORY ${PROJECT_SOURCE_DIR}/plugins      DESTINATION ${PLUGINS_INSTALL_DIR}      PATTERN .svn EXCLUDE)

# On Windows systems, install the libraries, Qt plugins and Qt translations
IF (WIN32)
	# Libraries
	foreach (lib ${INSTALL_LIBS})
		install (
			FILES ${PROJECT_BINARY_DIR}/${lib}
			DESTINATION ${EXECUTABLE_INSTALL_DIR})
	endforeach ()

	# Qt plugins
	install (
		DIRECTORY ${PROJECT_BINARY_DIR}/plugins
		DESTINATION ${QT_PLUGINS_INSTALL_DIR})

	# Qt translations
	install (
		DIRECTORY ${QT_TRANSLATIONS_DIR}/
		DESTINATION ${QT_TRANSLATIONS_INSTALL_DIR}
		FILES_MATCHING PATTERN qt_??.qm PATTERN qt_??_??.qm)
ENDIF ()

IF (NOT WIN32)
	INSTALL (FILES config/startkladde.desktop DESTINATION ${MENU_INSTALL_DIR})
	INSTALL (FILES graphics/startkladde.png   DESTINATION ${ICON_INSTALL_DIR})
ENDIF ()


###############
## Uninstall ##
###############

# uninstall target
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)

add_custom_target (
	uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)



#######################
## Package - generic ##
#######################

# Package information
set (CPACK_PACKAGE_NAME "Startkladde")
set (CPACK_PACKAGE_VERSION_MAJOR "${startkladde_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${startkladde_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${startkladde_VERSION_REVISION}")
SET (CPACK_PACKAGE_DESCRIPTION "Startkladde is a flight logging application for flying clubs.")
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "A flight logging application for flying clubs")
SET (CPACK_PACKAGE_VENDOR "Startkladde team")
SET (CPACK_PACKAGE_CONTACT "martin@famherrmann.de")
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/doc/gpl.txt")

# Installed files
SET(CPACK_PACKAGE_EXECUTABLES "startkladde" "Startkladde")

# Package settings
SET (CPACK_SOURCE_IGNORE_FILES CMakeLists.txt.user "~$" ".svn$")
SET (CPACK_STRIP_FILES 1)


####################
## Package - NSIS ##
####################

# Installer appearance
SET (CPACK_NSIS_DISPLAY_NAME "Startkladde")
#SET (CPACK_NSIS_MUI_ICON    ${PROJECT_SOURCE_DIR}/graphics\\\\startkladde.ico) # Installer icon
#SET (CPACK_NSIS_MUI_UNIICON ${PROJECT_SOURCE_DIR}/graphics\\\\startkladde.ico) # Uninstaller icon
#SET (CPACK_PACKAGE_ICON    "${PROJECT_SOURCE_DIR}/graphics\\\\test.bmp"      ) # Header image, 150x57

# Installer actions
SET (CPACK_NSIS_MODIFY_PATH off)                    # Do not add to PATH
SET (CPACK_NSIS_MUI_FINISHPAGE_RUN startkladde.exe) # Ask user to launch

# Start menu
SET (CPACK_NSIS_EXECUTABLES_DIRECTORY ".") # For menu entries
SET (CPACK_NSIS_INSTALLED_ICON_NAME startkladde.exe)

# Control panel information
SET (CPACK_NSIS_HELP_LINK      "http://startkladde.sourceforge.net")
SET (CPACK_NSIS_URL_INFO_ABOUT "http://startkladde.sourceforge.net")
SET (CPACK_NSIS_CONTACT        "martin@famherrmann.de")


######################
## Package - Debian ##
######################

SET (CPACK_DEBIAN_PACKAGE_NAME startkladde)
SET (CPACK_DEBIAN_PACKAGE_VERSION ${startkladde_VERSION_MAJOR}.${startkladde_VERSION_MINOR}.${startkladde_VERSION_REVISION})
#SET (CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)

# Do not explicitly require the MySQL client library. On Ubuntu, its name may
# changes between releases (e. g. libmysqlclient18 on Ubuntu Precise), which
# means that it has to be different for different Ubuntu releases. On the other
# hand, it is already required by libqt4-sql-mysql, so we don't have to specify
# it explicitly.
SET (CPACK_DEBIAN_PACKAGE_DEPENDS    "libc6 (>= 2.15), libgcc1 (>= 1:4.6.3), libstdc++6 (>= 4.6.3)")
SET (CPACK_DEBIAN_PACKAGE_DEPENDS    "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libqt4-network (>= 4:4.8.0), libqt4-sql (>= 4:4.8.0), libqt4-xml (>=4:4.8.0), libqtcore4 (>= 4:4.8.0), libqtgui4 (>= 4:4.8.0), libqt4-sql-mysql (>= 4:4.8.0)")
SET (CPACK_DEBIAN_PACKAGE_DEPENDS    "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libacpi0 (>=0.2)")
SET (CPACK_DEBIAN_PACKAGE_MAINTAINER "Martin Herrmann <martin@famherrmann.de>")
SET (CPACK_DEBIAN_PACKAGE_SECTION    "misc")
SET (CPACK_DEBIAN_PACKAGE_PRIORITY   "optional")
SET (CPACK_DEBIAN_PACKAGE_RECOMMENDS "mysql-server")
SET (CPACK_DEBIAN_PACKAGE_SUGGESTS   "kvkbd")
SET (CPACK_DEBIAN_PACKAGE_SUGGESTS   "kvkbd")

# TODO: the old (PPA) debian package included some files that are not longer
# present:
# /usr/share/menu/startkladde
# /usr/share/lintian/overrides/startkladde
# /usr/share/man/man1/startkladde.1.gz
# /usr/share/doc/startkladde/TODO.txt.gz
# /usr/share/doc/startkladde/changelog.gz
# /usr/share/doc/startkladde/compile.txt
# /usr/share/doc/startkladde/install.txt
# /usr/share/doc/startkladde/copyright

include (CPack)


#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
