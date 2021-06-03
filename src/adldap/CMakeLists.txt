find_package(Qt5 REQUIRED
    COMPONENTS
        Core
        LinguistTools
)

find_package(Ldap REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NdrStandard REQUIRED IMPORTED_TARGET ndr_standard)
pkg_check_modules(Smbclient REQUIRED IMPORTED_TARGET smbclient)
pkg_check_modules(Krb5 REQUIRED IMPORTED_TARGET krb5)
pkg_check_modules(Glib REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(Uuid REQUIRED IMPORTED_TARGET uuid)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_library(adldap SHARED
    ad_interface.cpp
    ad_config.cpp
    ad_utils.cpp
    ad_object.cpp
    ad_display.cpp
    ad_filter.cpp

    samba/gp_manage.cpp
    samba/ndr_security.c
    samba/dom_sid.c
    samba/replace.c
    samba/ndr_misc.c
    samba/libsmb_xattr.c
    samba/security_descriptor.c

    adldap.qrc
)

target_include_directories(adldap PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(adldap
    Qt5::Core
    PkgConfig::Uuid
    PkgConfig::Smbclient
    PkgConfig::Smbclient
    PkgConfig::Krb5
    Ldap::Ldap
    PkgConfig::NdrStandard
    resolv
)

set(TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/adldap_ru.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/adldap_en.ts
)

# NOTE: custom target to update translation sources (.ts).
# By default, translation sources aren't updated so you have
# to manually make this target to commit translation
# changes.
add_custom_target(adldap_ts
    DEPENDS ${TS_FILES}
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -recursive ${CMAKE_CURRENT_SOURCE_DIR} -ts ${TS_FILES} -noobsolete
)

# Generate translation binaries (.qm) from sources (.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR})
qt5_add_translation(QM_FILES ${TS_FILES})

install(TARGETS adldap)
