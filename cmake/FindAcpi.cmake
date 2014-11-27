# Tries to find libacpi.
# This module defines:
#  ACPI_INCLUDE_DIR, where to find libacpi.h
#  ACPI_LIBRARIES, the libraries needed to use libacpi (including dependencies)
#  ACPI_FOUND, if false, do not try to use libacpi
#  ACPI_LIBRARY, where to find the ACPI  library (without dependencies)

FIND_PATH (ACPI_INCLUDE_DIR libacpi.h)
FIND_LIBRARY (ACPI_LIBRARY NAMES acpi)

# handle the QUIETLY and REQUIRED arguments and set ACPI_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (ACPI DEFAULT_MSG ACPI_LIBRARY ACPI_INCLUDE_DIR)

IF (ACPI_FOUND)
  SET(ACPI_LIBRARIES ${ACPI_LIBRARY})
ENDIF ()

MARK_AS_ADVANCED(ACPI_LIBRARY ACPI_INCLUDE_DIR )
