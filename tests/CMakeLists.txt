
find_package(Qt5 REQUIRED
    COMPONENTS
        Core
        Widgets
        Test
        Help
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

link_libraries(
    Qt5::Core
    Qt5::Widgets
    Qt5::Test
    Qt5::Help
    adldap
    admctest
)

include_directories(
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src/admc
        ${PROJECT_SOURCE_DIR}/src/adldap
        ${PROJECT_SOURCE_DIR}/tests

        # NOTE: hack to get to generated .ui headers
        ${PROJECT_BINARY_DIR}/src/admc/admctest_autogen/include
)

# NOTE: ADD ALL TESTS TO THIS LIST
# Target name must equal to it's .cpp name
# target + target.cpp
set(TEST_TARGETS
    admc_test_ad_interface
    admc_test_unlock_edit
    admc_test_upn_edit
    admc_test_string_edit
    admc_test_string_large_edit
    admc_test_country_edit
    admc_test_gplink
    admc_test_select_base_widget
    admc_test_filter_widget
    admc_test_security_tab
    admc_test_attributes_tab
    admc_test_members_tab
    admc_test_member_of_tab
    admc_test_select_object_dialog
    admc_test_logon_hours_dialog
    admc_test_logon_computers_edit
    admc_test_expiry_edit
    admc_test_password_edit
    admc_test_group_scope_edit
    admc_test_group_type_edit
    admc_test_datetime_edit
    admc_test_manager_edit
    admc_test_delegation_edit
    admc_test_string_other_edit
    admc_test_account_option_edit
    admc_test_gpoptions_edit
    admc_test_protect_deletion_edit
    admc_test_octet_editor
    admc_test_bool_editor
    admc_test_datetime_editor
    admc_test_string_editor
    admc_test_multi_editor
    admc_test_edit_query_item_widget
    admc_test_policy_results_widget
    admc_test_find_object_dialog
    admc_test_rename_object_dialog
    admc_test_create_object_dialog
)

foreach(target ${TEST_TARGETS})
    add_executable(${target}
        admc_test.cpp
        ${target}.cpp
    )

    add_test(${target}
        ${PROJECT_BINARY_DIR}/${target}
    )

    install(TARGETS ${target} DESTINATION ${CMAKE_INSTALL_BINDIR}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endforeach()
