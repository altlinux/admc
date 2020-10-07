# Finds and exports uuid library as Uuid::Uuid

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_UUID uuid)
endif()

find_path(UUID_INCLUDE_DIR
    NAMES uuid/uuid.h
    HINTS ${PC_UUID_INCLUDEDIR}
)

find_library(UUID_LIBRARY
    NAMES uuid
    HINTS ${PC_UUID_LIBDIR}
)

mark_as_advanced(
    UUID_INCLUDE_DIR
    UUID_LIBRARY
)

find_package_handle_standard_args(Uuid
    FOUND_VAR UUID_FOUND
    REQUIRED_VARS
        UUID_INCLUDE_DIR
        UUID_LIBRARY
)

if(UUID_FOUND AND NOT TARGET Uuid::Uuid)
    add_library(Uuid::Uuid INTERFACE IMPORTED)

    target_link_libraries(Uuid::Uuid
        INTERFACE
            uuid
    )

    target_include_directories(Uuid::Uuid
        INTERFACE
            ${UUID_INCLUDE_DIR}
    )
endif()
