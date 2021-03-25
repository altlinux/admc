find_package(Qt5 REQUIRED
    COMPONENTS
        Core
        LinguistTools
)

find_package(Uuid REQUIRED)
find_package(Smbclient REQUIRED)
find_package(Krb5 REQUIRED)
find_package(Ldap REQUIRED)
find_package(Resolv REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(TS_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(TS_FILES
    ${TS_DIR}/adldap_ru.ts
    ${TS_DIR}/adldap_en.ts
)

set(QM_FILES
    ${TS_DIR}/adldap_ru.qm
    ${TS_DIR}/adldap_en.qm
)



add_library(adldap SHARED
    ad_interface.cpp
    ad_config.cpp
    ad_utils.cpp
    ad_object.cpp
    ad_display.cpp
    ad_filter.cpp

    adldap.qrc
)

target_include_directories(adldap PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(adldap
    Qt5::Core
    Uuid::Uuid
    Smbclient::Smbclient
    Krb5::Krb5
    Ldap::Ldap
    Resolv::Resolv
)

# Generate translation binaries (.qm) from sources (.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${TS_DIR})
qt5_add_translation(QM_FILES ${TS_FILES})

# NOTE: updating translations(.cpp -> .ts) is in a separate target so that it's not run every time source changes. When you do need to update translations, do `make update_translations`. *Generating* translations IS done every time (.ts -> .qm).

add_custom_target(update_adldap_translations
    DEPENDS ${TS_FILES}
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -recursive ${CMAKE_CURRENT_SOURCE_DIR} -ts ${TS_FILES} -noobsolete
)

add_custom_target(adldap_translations
    ALL
    DEPENDS ${QM_FILES}
)

install(TARGETS adldap)
