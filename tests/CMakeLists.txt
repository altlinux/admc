
find_package(Qt5 REQUIRED
    COMPONENTS
        Core
        Widgets
        Test
        Help
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(ADMC_APPLICATION_NAME "ADMC")
set(ADMC_APPLICATION_DISPLAY_NAME "ADMC")
set(ADMC_ORGANIZATION "BaseALT")
set(ADMC_ORGANIZATION_DOMAIN "basealt.ru")

# TODO: configure_file() call in admc's cmakelists is much cleaner, but doesn't work here. Fix this.
configure_file(${PROJECT_SOURCE_DIR}/src/admc/config.h.in "config.h")

set(TEST_SOURCES
    admc_test.cpp

    ${PROJECT_SOURCE_DIR}/src/admc/properties_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/main_window.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/status.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/select_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/console_actions.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/password_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/find_results.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/find_widget.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/tab_widget.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/central_widget.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/search_thread.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/help_browser.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/globals.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/utils.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/settings.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/policy_results_widget.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/gplink.cpp
    
    ${PROJECT_SOURCE_DIR}/src/admc/console_types/console_object.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/console_types/console_policy.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/console_types/console_query.cpp

    ${PROJECT_SOURCE_DIR}/src/admc/move_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/rename_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/rename_policy_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/create_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/find_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/find_select_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/select_container_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/about_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/manual_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/select_policy_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/create_policy_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/create_query_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/create_query_folder_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/edit_query_folder_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/move_query_dialog.cpp

    ${PROJECT_SOURCE_DIR}/src/admc/filter_widget/filter_widget.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_widget/filter_widget_simple_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_widget/filter_widget_normal_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_widget/filter_widget_advanced_tab.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_widget/select_classes_widget.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_widget/filter_builder.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/filter_widget/search_base_widget.cpp
    
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
    ${PROJECT_SOURCE_DIR}/src/admc/tabs/managed_by_tab.cpp

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
    ${PROJECT_SOURCE_DIR}/src/admc/edits/upn_edit.cpp

    ${PROJECT_SOURCE_DIR}/src/admc/console_widget/console_widget.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/console_widget/scope_model.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/console_widget/customize_columns_dialog.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/console_widget/results_description.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/console_widget/results_view.cpp
    ${PROJECT_SOURCE_DIR}/src/admc/console_widget/console_drag_model.cpp

    ${PROJECT_SOURCE_DIR}/src/admc/admc.qrc
)

# NOTE: ADD ALL TESTS TO THIS LIST
# Target name must equal to it's .cpp name
# target + target.cpp
set(TEST_TARGETS
    admc_test_ad_interface
    admc_test_object_menu
    admc_test_unlock_edit
    admc_test_upn_edit
    admc_test_string_edit
    admc_test_country_edit
    admc_test_gplink
)

foreach(target ${TEST_TARGETS})
    add_executable(${target}
        ${target}.cpp

        ${TEST_SOURCES}
    )

    add_test(${target}
        ${PROJECT_BINARY_DIR}/${target}
    )

    target_include_directories(${target}
        PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${PROJECT_SOURCE_DIR}/src/admc
            ${PROJECT_SOURCE_DIR}/tests
            ${PROJECT_SOURCE_DIR}/src/adldap
    )

    target_link_libraries(${target}
        Qt5::Core
        Qt5::Widgets
        Qt5::Test
        Qt5::Help
        adldap
        resolv
    )

    install(TARGETS ${target})
endforeach()
