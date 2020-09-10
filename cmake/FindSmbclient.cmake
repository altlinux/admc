# Finds smbclient library
#
# Defined vars:
#     Smbclient_FOUND
#     Smbclient_INCLUDE_DIRS
# 
# Imported targets:
#     Smbclient::Smbclient

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_SAMBA smbclient)
endif()

find_path(Smbclient_INCLUDE_DIR
    NAMES libsmbclient.h
    HINTS ${PC_SAMBA_INCLUDEDIR}
)

find_library(Smbclient_LIBRARY
    NAMES smbclient
    HINTS ${PC_SAMBA_LIBDIR}
)

mark_as_advanced(
    Smbclient_INCLUDE_DIR
    Smbclient_LIBRARY
)

find_package_handle_standard_args(Smbclient
    FOUND_VAR Smbclient_FOUND
    REQUIRED_VARS
        Smbclient_INCLUDE_DIR
        Smbclient_LIBRARY
)

if(Smbclient_FOUND)
    set(Smbclient_INCLUDE_DIRS ${Smbclient_INCLUDE_DIR})
endif()

if(Smbclient_FOUND AND NOT TARGET Smbclient::Smbclient)
    add_library(Smbclient::Smbclient INTERFACE IMPORTED)

    target_link_libraries(Smbclient::Smbclient
        INTERFACE
            smbclient
    )

    target_include_directories(Smbclient::Smbclient
        INTERFACE
            ${Smbclient_INCLUDE_DIRS}
    )
endif()
