# Finds LDAP C Libraries
#
# Defined vars:
#     Ldap_FOUND
#     Ldap_INCLUDE_DIRS
# 
# Imported targets:
#     Ldap::Ldap

find_path(LDAP_INCLUDE_DIR
    NAMES ldap.h
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
)
find_library(LDAP_LIBRARY ldap)

find_path(LBER_INCLUDE_DIR
    NAMES lber.h
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
)
find_library(LBER_LIBRARY lber)

mark_as_advanced(
    LDAP_INCLUDE_DIR
    LBER_INCLUDE_DIR
    LDAP_LIBRARY
    LBER_LIBRARY
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ldap
    FOUND_VAR Ldap_FOUND
    REQUIRED_VARS
        LDAP_INCLUDE_DIR
        LBER_INCLUDE_DIR
        LDAP_LIBRARY
        LBER_LIBRARY
)

if(Ldap_FOUND)
    set(Ldap_INCLUDE_DIRS ${LDAP_INCLUDE_DIR} ${LBER_INCLUDE_DIR})
    list(REMOVE_DUPLICATES Ldap_INCLUDE_DIRS)
endif()

if(Ldap_FOUND AND NOT TARGET Ldap::Ldap)
    add_library(Ldap::Ldap INTERFACE IMPORTED)
    
    target_link_libraries(Ldap::Ldap
        INTERFACE
            ldap
            lber
    )

    target_include_directories(Ldap::Ldap
        INTERFACE
            ${Ldap_INCLUDE_DIRS}
    )
endif()
