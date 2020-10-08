# Finds Resolv library contained in Glib package
# NOTE: Glib requires libpcre

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_UUID glib-2.0)
endif()

find_path(RESOLV_INCLUDE_DIR
    NAMES resolv.h
    HINTS ${PC_GLIB_INCLUDEDIR}
)
find_library(RESOLV_LIBRARY
    resolv
    HINTS ${PC_GLIB_LIBDIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Glib
    FOUND_VAR Glib_FOUND
    REQUIRED_VARS
        RESOLV_INCLUDE_DIR
        RESOLV_LIBRARY
)

if(Glib_FOUND)
    set(Resolv_INCLUDE_DIRS ${RESOLV_INCLUDE_DIR})
    list(REMOVE_DUPLICATES Resolv_INCLUDE_DIRS)
endif()

if(Glib_FOUND AND NOT TARGET Resolv::Resolv)
    add_library(Resolv::Resolv INTERFACE IMPORTED)
    
    target_link_libraries(Resolv::Resolv
        INTERFACE
            resolv
    )

    target_include_directories(Resolv::Resolv
        INTERFACE
            ${Resolv_INCLUDE_DIRS}
    )
endif()
