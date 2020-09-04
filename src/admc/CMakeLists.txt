find_package(Qt5 REQUIRED COMPONENTS
    Core
    Widgets
    LinguistTools
    )

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Definitions for configuration file
set(ADMC_APPLICATION_NAME "ADMC")
set(ADMC_APPLICATION_DISPLAY_NAME "ADMC")
set(ADMC_ORGANIZATION "BaseALT")
set(ADMC_ORGANIZATION_DOMAIN "basealt.ru")

configure_file("config.h.in" "config.h")

set(TS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/translations)

set(TS_FILES
    ${TS_DIR}/ru.ts
    ${TS_DIR}/en.ts
    )

# Do `make update_admc_translations` to update translation sources (.ts)
# NOTE: this is in a custom target so that translation sources aren't rebuilt everytime app source changes
add_custom_target(update_admc_translations
    DEPENDS ${TS_FILES}
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR} -ts ${TS_FILES}
    )

# Generate translation binaries (.qm) from sources (.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${TS_DIR})
qt5_add_translation(QM_FILES ${TS_FILES})

add_executable(admc
    main.cpp
    ad_interface.cpp
    details_widget.cpp
    details_tab.cpp
    attributes_tab.cpp
    containers_widget.cpp
    contents_widget.cpp
    main_window.cpp
    status.cpp
    members_tab.cpp
    object_model.cpp
    settings.cpp
    select_dialog.cpp
    object_context_menu.cpp
    confirmation_dialog.cpp
    advanced_view_proxy.cpp
    dn_column_proxy.cpp
    utils.cpp
    login_dialog.cpp
    password_dialog.cpp
    menubar.cpp
    account_tab.cpp
    general_tab.cpp
    address_tab.cpp
    object_tab.cpp
    rename_dialog.cpp
    create_dialog.cpp
    attribute_display_strings.cpp
    
    edits/attribute_edit.cpp
    edits/string_edit.cpp
    edits/country_edit.cpp
    edits/expiry_edit.cpp
    edits/unlock_edit.cpp
    edits/group_scope_edit.cpp
    edits/group_type_edit.cpp
    edits/account_option_edit.cpp
    edits/password_edit.cpp
    edits/datetime_edit.cpp

    admc.qrc
    translations/translations.qrc
    )

target_include_directories(admc
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

target_link_libraries(admc
    Qt5::Core
    Qt5::Widgets
    adldap++
    )

install(TARGETS admc)
