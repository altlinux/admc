
find_package(Qt5 REQUIRED
    COMPONENTS
        Core
        Widgets
        Test
)

find_package(Uuid REQUIRED)
find_package(Smbclient REQUIRED)
find_package(Krb5 REQUIRED)
find_package(Ldap REQUIRED)
find_package(Resolv REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(ADMC_APPLICATION_NAME "ADMC")
set(ADMC_APPLICATION_DISPLAY_NAME "ADMC")
set(ADMC_ORGANIZATION "BaseALT")
set(ADMC_ORGANIZATION_DOMAIN "basealt.ru")

# TODO: configure_file() call in admc's cmakelists is much cleaner, but doesn't work here. Fix this.
configure_file(${PROJECT_SOURCE_DIR}/src/admc/config.h.in "config.h")

enable_testing()

# TODO: not sure how to avoid duplicating the list of ADMC sources here. Maybe could make a shared list somehow? Also maybe move "tests" so that it's inside "src/admc".
add_executable(test_admc
    test_admc.cpp

    ${PROJECT_SOURCE_DIR}/src/admc/ad_interface.cpp
    
    ${PROJECT_SOURCE_DIR}/src/admc/ad_config.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/ad_utils.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/ad_object.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/properties_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/main_window.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/status.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/object_model.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/select_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/object_menu.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/confirmation_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/password_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/menubar.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/attribute_display.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/find_results.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/toggle_widgets_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/find_widget.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tab_widget.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/console.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/console_drag_model.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/rename_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/rename_policy_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/create_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/find_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/find_select_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/select_container_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/policies_widget.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/gplink.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_widget/filter_widget.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_widget/filter_widget_simple_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_widget/filter_widget_normal_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_widget/filter_widget_advanced_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_widget/select_classes_widget.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_widget/filter_builder.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tabs/properties_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tabs/general_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tabs/object_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tabs/attributes_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tabs/account_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tabs/membership_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tabs/address_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tabs/group_policy_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tabs/gpo_links_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tabs/organization_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tabs/telephones_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tabs/profile_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/editors/attribute_editor.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/editors/multi_editor.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/editors/string_editor.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/editors/octet_editor.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/editors/bool_editor.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/editors/datetime_editor.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/attribute_edit.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/string_edit.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/string_other_edit.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/string_large_edit.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/country_edit.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/expiry_edit.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/unlock_edit.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/group_scope_edit.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/group_type_edit.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/account_option_edit.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/password_edit.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/datetime_edit.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/gpoptions_edit.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edits/manager_edit.cpp

    ${PROJECT_SOURCE_DIR}/src/common/utils.cpp
    ${PROJECT_SOURCE_DIR}/src/common/settings.cpp
    ${PROJECT_SOURCE_DIR}/src/common/filter.cpp

    ${PROJECT_SOURCE_DIR}/src/admc/admc.qrc
    ${PROJECT_SOURCE_DIR}/translations/translations.qrc
)

add_test(test_admc
    test_admc
)

target_include_directories(test_admc
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src/common
        ${PROJECT_SOURCE_DIR}/src/admc
        ${PROJECT_SOURCE_DIR}/tests
)

target_link_libraries(test_admc
    Qt5::Core
    Qt5::Widgets
    Qt5::Test
    Uuid::Uuid
    Smbclient::Smbclient
    Krb5::Krb5
    Ldap::Ldap
    Resolv::Resolv
)
