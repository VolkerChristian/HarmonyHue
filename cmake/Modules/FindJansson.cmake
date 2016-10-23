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
pkg_check_modules(Jansson_PKGCONF REQUIRED jansson)

# Include dir
find_path(Jansson_INCLUDE_DIR
  NAMES jansson.h
  PATHS ${Jansson_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(Jansson_LIBRARY
  NAMES jansson
  PATHS ${Jansson_PKGCONF_LIBRARY_DIRS}
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Jansson DEFAULT_MSG Jansson_LIBRARY Jansson_INCLUDE_DIR)

if(Jansson_PKGCONF_FOUND)
  set(JANSSON_LIBRARIES ${Jansson_LIBRARY})# ${Jansson_PKGCONF_LIBRARIES})
  set(JANSSON_INCLUDE_DIRS ${Jansson_INCLUDE_DIR} ${Jansson_PKGCONF_INCLUDE_DIRS})
  set(JANSSON_FOUND yes)
else()
  set(JANSSON_LIBRARIES)
  set(JANSSON_INCLUDE_DIRS)
  set(JANSSON_FOUND no)
endif()

# if (JANSSON_LIBRARIES AND JANSSON_INCLUDE_DIRS)
#   # in cache already
#   set(JANSSON_FOUND TRUE)
# else (JANSSON_LIBRARIES AND JANSSON_INCLUDE_DIRS)
#   find_path(JANSSON_INCLUDE_DIR
#     NAMES
#       jansson.h
#     PATHS
#       /usr/include
#       /usr/local/include
#       /opt/local/include
#       /sw/include
#   )
# 
# find_library(JANSSON_LIBRARY
#     NAMES
#       jansson
#     PATHS
#       /usr/lib
#       /usr/local/lib
#       /opt/local/lib
#       /sw/lib
#   )
# 
# set(JANSSON_INCLUDE_DIRS
#   ${JANSSON_INCLUDE_DIR}
#   )
# 
# if (JANSSON_LIBRARY)
#   set(JANSSON_LIBRARIES
#     ${JANSSON_LIBRARIES}
#     ${JANSSON_LIBRARY}
#     )
# endif (JANSSON_LIBRARY)
# 
#   include(FindPackageHandleStandardArgs)
#   find_package_handle_standard_args(Jansson DEFAULT_MSG
#     JANSSON_LIBRARIES JANSSON_INCLUDE_DIRS)
# 
#   # show the JANSSON_INCLUDE_DIRS and JANSSON_LIBRARIES variables only in the advanced view
#   mark_as_advanced(JANSSON_INCLUDE_DIRS JANSSON_LIBRARIES)
# 
# endif (JANSSON_LIBRARIES AND JANSSON_INCLUDE_DIRS)


