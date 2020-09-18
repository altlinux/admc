find_package(Qt5 REQUIRED
    COMPONENTS
        Core
        Widgets
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(ADMC_APPLICATION_NAME "ADMC")
set(ADMC_APPLICATION_DISPLAY_NAME "ADMC")
set(ADMC_ORGANIZATION "BaseALT")
set(ADMC_ORGANIZATION_DOMAIN "basealt.ru")
configure_file("config.h.in" "config.h")

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
    login_dialog.cpp
    password_dialog.cpp
    menubar.cpp
    account_tab.cpp
    general_tab.cpp
    address_tab.cpp
    object_tab.cpp
    group_policy_tab.cpp
    gpo_links_tab.cpp
    rename_dialog.cpp
    create_dialog.cpp
    attribute_display_strings.cpp
    policies_widget.cpp
    
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
    edits/gpoptions_edit.cpp

    ${PROJECT_SOURCE_DIR}/src/common/utils.cpp

    admc.qrc
    ${PROJECT_SOURCE_DIR}/translations/translations.qrc
)

target_include_directories(admc
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src/common
)

target_link_libraries(admc
    Qt5::Core
    Qt5::Widgets
    adldap
)

install(TARGETS admc)
