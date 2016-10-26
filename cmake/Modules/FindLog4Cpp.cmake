# - Try to find Jansson
# Once done this will define
#
#  JANSSON_FOUND - system has Jansson
#  JANSSON_INCLUDE_DIRS - the Jansson include directory
#  JANSSON_LIBRARIES - Link these to use Jansson
#
#  Copyright (c) 2011 Lee Hambley <lee.hambley@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(FindPkgConfig)
include(FindPackageHandleStandardArgs)

# Use pkg-config to get hints about paths
pkg_check_modules(Log4Cpp_PKGCONF REQUIRED log4cpp)

# Include dir
find_path(Log4Cpp_INCLUDE_DIR
  NAMES log4cpp/Appender.hh
  PATHS ${Log4Cpp_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(Log4Cpp_LIBRARY
  NAMES log4cpp
  PATHS ${Log4Cpp_PKGCONF_LIBRARY_DIRS}
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Log4Cpp DEFAULT_MSG Log4Cpp_LIBRARY Log4Cpp_INCLUDE_DIR)

if(Jansson_PKGCONF_FOUND)
  set(Log4Cpp_LIBRARIES ${Log4Cpp_LIBRARY}) #${Log4Cpp_PKGCONF_LIBRARIES})
  set(Log4Cpp_INCLUDE_DIRS ${Log4Cpp_INCLUDE_DIR} ${Log4Cpp_PKGCONF_INCLUDE_DIRS})
  set(Log4Cpp_FOUND yes)
else()
  set(Log4Cpp_LIBRARIES)
  set(Log4Cpp_INCLUDE_DIRS)
  set(Log4Cpp_FOUND no)
endif()

