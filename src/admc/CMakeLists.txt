find_package(Qt5 REQUIRED
    COMPONENTS
        Core
        Widgets
        Help
        LinguistTools
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(ADMC_APPLICATION_NAME "ADMC")
set(ADMC_APPLICATION_DISPLAY_NAME "ADMC")
set(ADMC_ORGANIZATION "BaseALT")
set(ADMC_ORGANIZATION_DOMAIN "basealt.ru")
configure_file("config.h.in" "config.h")

include_directories(
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src/adldap
)

link_libraries(
    Qt5::Core
    Qt5::Widgets
    Qt5::Help
    adldap
    resolv
)

set(ADMC_SOURCES
    status.cpp
    search_thread.cpp
    globals.cpp
    utils.cpp
    settings.cpp
    widget_state.cpp

    main_window.cpp
    find_widget.cpp
    tab_widget.cpp
    help_browser.cpp
    policy_results_widget.cpp
    filter_classes_widget.cpp
    edit_query_item_widget.cpp

    console_filter_dialog.cpp
    password_dialog.cpp
    rename_object_dialog.cpp
    rename_other_dialog.cpp
    rename_policy_dialog.cpp
    rename_user_dialog.cpp
    rename_group_dialog.cpp
    create_object_dialog.cpp
    create_user_dialog.cpp
    create_group_dialog.cpp
    create_computer_dialog.cpp
    create_ou_dialog.cpp
    create_policy_dialog.cpp
    create_query_item_dialog.cpp
    create_query_folder_dialog.cpp
    find_object_dialog.cpp
    select_object_dialog.cpp
    select_object_advanced_dialog.cpp
    select_container_dialog.cpp
    select_policy_dialog.cpp
    about_dialog.cpp
    manual_dialog.cpp
    edit_query_item_dialog.cpp
    edit_query_folder_dialog.cpp
    properties_dialog.cpp
    properties_multi_dialog.cpp
    properties_warning_dialog.cpp
    change_dc_dialog.cpp
    connection_options_dialog.cpp
    changelog_dialog.cpp
    error_log_dialog.cpp

    filter_widget/filter_widget.cpp
    filter_widget/filter_dialog.cpp
    filter_widget/filter_widget_simple_tab.cpp
    filter_widget/filter_widget_normal_tab.cpp
    filter_widget/filter_widget_advanced_tab.cpp
    filter_widget/select_classes_widget.cpp
    filter_widget/select_classes_dialog.cpp
    filter_widget/select_base_widget.cpp
    
    tabs/properties_tab.cpp
    tabs/general_user_tab.cpp
    tabs/general_group_tab.cpp
    tabs/general_ou_tab.cpp
    tabs/general_computer_tab.cpp
    tabs/general_policy_tab.cpp
    tabs/general_other_tab.cpp
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
    tabs/managed_by_tab.cpp
    tabs/security_tab.cpp
    tabs/os_tab.cpp
    tabs/delegation_tab.cpp
    tabs/select_well_known_trustee_dialog.cpp

    multi_tabs/properties_multi_tab.cpp
    multi_tabs/general_user_multi_tab.cpp
    multi_tabs/general_other_multi_tab.cpp
    multi_tabs/account_multi_tab.cpp
    multi_tabs/address_multi_tab.cpp
    multi_tabs/profile_multi_tab.cpp
    multi_tabs/organization_multi_tab.cpp

    edits/attribute_edit.cpp
    edits/string_edit.cpp
    edits/string_other_edit.cpp
    edits/string_large_edit.cpp
    edits/sam_name_edit.cpp
    edits/country_edit.cpp
    edits/expiry_edit.cpp
    edits/expiry_widget.cpp
    edits/unlock_edit.cpp
    edits/group_scope_edit.cpp
    edits/group_type_edit.cpp
    edits/account_option_edit.cpp
    edits/password_edit.cpp
    edits/datetime_edit.cpp
    edits/gpoptions_edit.cpp
    edits/manager_edit.cpp
    edits/upn_edit.cpp
    edits/upn_suffix_combo.cpp
    edits/country_combo.cpp
    edits/manager_widget.cpp
    edits/delegation_edit.cpp
    edits/logon_hours_edit.cpp
    edits/logon_computers_edit.cpp
    edits/protect_deletion_edit.cpp

    multi_edits/attribute_multi_edit.cpp
    multi_edits/string_multi_edit.cpp
    multi_edits/expiry_multi_edit.cpp
    multi_edits/upn_multi_edit.cpp
    multi_edits/account_option_multi_edit.cpp
    multi_edits/country_multi_edit.cpp
    multi_edits/manager_multi_edit.cpp

    editors/attribute_editor.cpp
    editors/multi_editor.cpp
    editors/string_editor.cpp
    editors/octet_editor.cpp
    editors/bool_editor.cpp
    editors/datetime_editor.cpp

    console_widget/console_widget.cpp
    console_widget/scope_proxy_model.cpp
    console_widget/customize_columns_dialog.cpp
    console_widget/results_view.cpp
    console_widget/console_drag_model.cpp
    console_widget/console_impl.cpp

    console_impls/object_impl.cpp
    console_impls/policy_impl.cpp
    console_impls/query_item_impl.cpp
    console_impls/query_folder_impl.cpp
    console_impls/policy_root_impl.cpp

    admc.qrc
)

add_executable(admc
    main.cpp
    ${ADMC_SOURCES}
)

# NOTE: build admc as library to streamline building tests.
# Tests just link to this lib and use all of admc's code.
add_library(admctest SHARED
    ${ADMC_SOURCES}
)

set(TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/admc_ru.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/admc_en.ts
)

# NOTE: custom target to update translation sources (.ts).
# By default, translation sources aren't updated so you have
# to manually make this target to commit translation
# changes.
add_custom_target(admc_ts
    DEPENDS ${TS_FILES}
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -recursive ${CMAKE_CURRENT_SOURCE_DIR} -ts ${TS_FILES} -noobsolete
)

# Generate translation binaries (.qm) from sources (.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR})
qt5_add_translation(QM_FILES ${TS_FILES})

install(TARGETS admc DESTINATION ${CMAKE_INSTALL_BINDIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
if(NOT ADMC_BUILD_DEB)
    install(TARGETS admctest DESTINATION ${CMAKE_INSTALL_LIBDIR}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif(NOT ADMC_BUILD_DEB)
