#.rst:
# FindSQLite3
# -----------
# Finds the sqlite3 library
#
# This will define the following variables::
#
# SQLITE3_FOUND - system has sqlite3 parser
# SQLITE3_INCLUDE_DIRS - the sqlite3 parser include directory
#

find_package(PkgConfig)

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_SQLITE3 sqlite3 QUIET)
endif()

find_path(SQLITE3_INCLUDE_DIR sqlite3.h PATHS ${PC_SQLITE3_INCLUDEDIR})
find_library(SQLITE3_LIBRARY NAMES sqlite3 PATHS ${PC_SQLITE3_LIBDIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(sqlite3
                                  REQUIRED_VARS SQLITE3_INCLUDE_DIR SQLITE3_VERSION
                                  VERSION_VAR SQLITE3_VERSION)

if(SQLITE3_FOUND)
  set(SQLITE3_INCLUDE_DIRS ${SQLITE3_INCLUDE_DIR})
endif()

mark_as_advanced(SQLITE3_INCLUDE_DIR)

