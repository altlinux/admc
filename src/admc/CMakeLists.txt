find_package(Qt5 REQUIRED
    COMPONENTS
        Core
        Widgets
)

find_package(Uuid REQUIRED)
find_package(Smbclient REQUIRED)
find_package(Krb5 REQUIRED)

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
    ad_config.cpp
    ad_utils.cpp
    ad_object.cpp
    details_dialog.cpp
    containers_widget.cpp
    contents_widget.cpp
    main_window.cpp
    status.cpp
    object_model.cpp
    select_dialog.cpp
    object_context_menu.cpp
    confirmation_dialog.cpp
    password_dialog.cpp
    menubar.cpp
    attribute_display.cpp
    object_list_widget.cpp
    toggle_widgets_dialog.cpp
    
    rename_dialog.cpp
    rename_policy_dialog.cpp
    create_dialog.cpp
    find_dialog.cpp
    policies_widget.cpp
    gplink.cpp

    filter_widget/filter_widget.cpp
    filter_widget/filter_widget_simple_tab.cpp
    filter_widget/filter_widget_normal_tab.cpp
    filter_widget/filter_widget_advanced_tab.cpp
    filter_widget/select_classes_widget.cpp
    filter_widget/filter_builder.cpp
    
    tabs/details_tab.cpp
    tabs/general_tab.cpp
    tabs/object_tab.cpp
    tabs/attributes_tab.cpp
    tabs/account_tab.cpp
    tabs/membership_tab.cpp
    tabs/address_tab.cpp
    tabs/group_policy_tab.cpp
    tabs/gpo_links_tab.cpp
    tabs/organization_tab.cpp
    tabs/telephones_tab.cpp
    tabs/profile_tab.cpp

    edit_dialogs/edit_dialog.cpp
    edit_dialogs/string_edit_dialog.cpp
    edit_dialogs/string_multi_edit_dialog.cpp
    edit_dialogs/binary_edit_dialog.cpp

    edits/attribute_edit.cpp
    edits/string_edit.cpp
    edits/string_other_edit.cpp
    edits/string_large_edit.cpp
    edits/country_edit.cpp
    edits/expiry_edit.cpp
    edits/unlock_edit.cpp
    edits/group_scope_edit.cpp
    edits/group_type_edit.cpp
    edits/account_option_edit.cpp
    edits/password_edit.cpp
    edits/datetime_edit.cpp
    edits/gpoptions_edit.cpp
    edits/manager_edit.cpp

    ${PROJECT_SOURCE_DIR}/src/common/utils.cpp
    ${PROJECT_SOURCE_DIR}/src/common/settings.cpp
    ${PROJECT_SOURCE_DIR}/src/common/filter.cpp

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
    Uuid::Uuid
    Smbclient::Smbclient
    Krb5::Krb5
)

install(TARGETS admc)
