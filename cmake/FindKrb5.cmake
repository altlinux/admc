find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_KRB5 krb5)
endif()

find_path(KRB5_INCLUDE_DIR
    NAMES krb5.h
    HINTS ${PC_KRB5_INCLUDEDIR}
)

find_library(KRB5_LIBRARY
    NAMES krb5
    HINTS ${PC_KRB5_LIBDIR}
)

mark_as_advanced(
    KRB5_INCLUDE_DIR
    KRB5_LIBRARY
)

find_package_handle_standard_args(Krb5
    FOUND_VAR KRB5_FOUND
    REQUIRED_VARS
        KRB5_INCLUDE_DIR
        KRB5_LIBRARY
)

if(KRB5_FOUND AND NOT TARGET Krb5::Krb5)
    add_library(Krb5::Krb5 INTERFACE IMPORTED)

    target_link_libraries(Krb5::Krb5
        INTERFACE
            krb5
    )

    target_include_directories(Krb5::Krb5
        INTERFACE
            ${KRB5_INCLUDE_DIR}
    )
endif()
