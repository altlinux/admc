# - Try to find the samba directory library
# Once done this will define
#
#  SAMBA_FOUND - system has SAMBA
#  SAMBA_INCLUDE_DIR - the SAMBA include directory
#  SAMBA_LIBRARIES - The libraries needed to use SAMBA

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(SAMBA_INCLUDE_DIR AND SAMBA_LIBRARIES)
    # Already in cache, be silent
    set(Samba_FIND_QUIETLY TRUE)
endif()

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_SAMBA smbclient)
endif()

find_path(SAMBA_INCLUDE_DIR NAMES libsmbclient.h HINTS ${PC_SAMBA_INCLUDEDIR})
find_library(SAMBA_LIBRARIES NAMES smbclient HINTS ${PC_SAMBA_LIBDIR})

if(SAMBA_INCLUDE_DIR AND SAMBA_LIBRARIES)
    set(SAMBA_FOUND TRUE)
else()
    set(SAMBA_FOUND FALSE)
endif()

if(SAMBA_FOUND)
    if(NOT Samba_FIND_QUIETLY)
        message(STATUS "Found samba: ${SAMBA_LIBRARIES}")
    endif()

    add_definitions(${PC_SAMBA_CFLAGS})
else()
    if (Samba_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find Samba library")
    endif()
endif()
