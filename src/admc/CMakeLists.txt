find_package(Qt6 REQUIRED
    COMPONENTS
        Core
        Widgets
        LinguistTools
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_library(LIBCNG_DPAPI_LIBRARY libcng-dpapi.so)

set(ADMC_APPLICATION_NAME "ADMC")
set(ADMC_APPLICATION_DISPLAY_NAME "ADMC")
set(ADMC_ORGANIZATION "BaseALT")
set(ADMC_ORGANIZATION_DOMAIN "basealt.ru")

if (LIBCNG_DPAPI_LIBRARY)
    set(ADMC_ENABLE_NATIVE_LAPS 1)
else()
    set(ADMC_ENABLE_NATIVE_LAPS 0)
endif()

configure_file("core/config.h.in" "core/config.h")

include_directories(
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src/adldap
)

find_library(RESOLV_LIBRARY libresolv.so)

link_libraries(
    Qt6::Core
    Qt6::Widgets
    adldap
)


if (RESOLV_LIBRARY)
    link_libraries(${RESOLV_LIBRARY})
endif()


if (LIBCNG_DPAPI_LIBRARY)
    message("Found libcng-dpapi library in ${LIBCNG_DPAPI_LIBRARY} .")
    link_libraries(${LIBCNG_DPAPI_LIBRARY})
    set(LAPS_SOURCES
        ui/widget/tab/laps_v2.cpp
        attribute_edits/laps_encrypted_attribute_edit.cpp
    )
else()
    message("Skipping libcng-dpapi library.")
    set(LAPS_SOURCES "")
endif()

set(ADMC_SOURCES
    core/ad.cpp
    core/admc_translator.cpp
    core/attribute.cpp
    core/changelog.cpp
    core/fsmo.cpp
    core/globals.cpp
    core/managers/country_manager.cpp
    core/managers/gplink_manager.cpp
    core/managers/icon_manager.cpp
    core/search_thread.cpp
    core/settings.cpp
    core/utils.cpp
    ui/dialog/about.cpp
    ui/dialog/attribute/attribute.cpp
    ui/dialog/attribute/bool.cpp
    ui/dialog/attribute/datetime.cpp
    ui/dialog/attribute/hex_number.cpp
    ui/dialog/attribute/list.cpp
    ui/dialog/attribute/number.cpp
    ui/dialog/attribute/octet.cpp
    ui/dialog/attribute/string.cpp
    ui/dialog/attribute/time_span.cpp
    ui/dialog/auth/krb_auth.cpp
    ui/dialog/changelog.cpp
    ui/dialog/class_filter.cpp
    ui/dialog/connection_options.cpp
    ui/dialog/console_filter.cpp
    ui/dialog/create/computer.cpp
    ui/dialog/create/contact.cpp
    ui/dialog/create/group.cpp
    ui/dialog/create/ou.cpp
    ui/dialog/create/policy.cpp
    ui/dialog/create/pso.cpp
    ui/dialog/create/query_folder.cpp
    ui/dialog/create/query_item.cpp
    ui/dialog/create/shared_folder.cpp
    ui/dialog/create/site.cpp
    ui/dialog/create/sites_link.cpp
    ui/dialog/create/subnet.cpp
    ui/dialog/create/user.cpp
    ui/dialog/edit_query_item.cpp
    ui/dialog/edit_query_folder.cpp
    ui/dialog/error_log.cpp
    ui/dialog/filter.cpp
    ui/dialog/find/policy.cpp
    ui/dialog/find/object.cpp
    ui/dialog/main_window_connection_error.cpp
    ui/dialog/password.cpp
    ui/dialog/properties.cpp
    ui/dialog/properties_multi.cpp
    ui/dialog/properties_warning.cpp
    ui/dialog/rename/group.cpp
    ui/dialog/rename/other.cpp
    ui/dialog/rename/policy.cpp
    ui/dialog/rename/user.cpp
    ui/dialog/security_sort_warning.cpp
    ui/dialog/select/object.cpp
    ui/dialog/select/object_match.cpp
    ui/dialog/select/object_advanced.cpp
    ui/dialog/select/container.cpp
    ui/dialog/select/policy.cpp
    ui/dialog/select/well_known_trustee.cpp
    ui/status.cpp
    ui/widget/console/console_widget.cpp
    ui/widget/console/scope_proxy_model.cpp
    ui/widget/console/customize_columns_dialog.cpp
    ui/widget/console/results_view.cpp
    ui/widget/console/console_drag_model.cpp
    ui/widget/console/console_impl.cpp
    ui/widget/edit_query_item.cpp
    ui/widget/filter/advanced_tab.cpp
    ui/widget/filter/class_filter.cpp
    ui/widget/filter/filter.cpp
    ui/widget/filter/normal_tab.cpp
    ui/widget/filter/select_base.cpp
    ui/widget/filter/select_classes.cpp
    ui/widget/filter/simple_tab.cpp
    ui/widget/find.cpp
    ui/widget/multi_tab/general_user.cpp
    ui/widget/multi_tab/general_other.cpp
    ui/widget/multi_tab/account.cpp
    ui/widget/multi_tab/address.cpp
    ui/widget/multi_tab/profile.cpp
    ui/widget/multi_tab/organization.cpp
    ui/widget/result/base.cpp
    ui/widget/result/domain_info/domain_info.cpp
    ui/widget/result/domain_info/fsmo_table.cpp
    ui/widget/result/policy.cpp
    ui/widget/result/policy_ou/drag_drop_links_model.cpp
    ui/widget/result/policy_ou/inherited_policies.cpp
    ui/widget/result/policy_ou/linked_policies.cpp
    ui/widget/result/policy_ou/policy_ou.cpp
    ui/widget/result/pso/pso.cpp
    ui/widget/result/pso/pso_edit.cpp
    ui/widget/result/subnet/subnet.cpp
    ui/widget/result/subnet/subnet_edit.cpp
    ui/widget/tab.cpp
    ui/widget/tab/general_user.cpp
    ui/widget/tab/general_group.cpp
    ui/widget/tab/general_ou.cpp
    ui/widget/tab/general_computer.cpp
    ui/widget/tab/general_policy.cpp
    ui/widget/tab/general_shared_folder.cpp
    ui/widget/tab/general_other.cpp
    ui/widget/tab/object.cpp
    ui/widget/tab/attributes.cpp
    ui/widget/tab/attributes_proxy.cpp
    ui/widget/tab/attributes_filter_menu.cpp
    ui/widget/tab/account.cpp
    ui/widget/tab/membership.cpp
    ui/widget/tab/address.cpp
    ui/widget/tab/organization.cpp
    ui/widget/tab/telephones.cpp
    ui/widget/tab/profile.cpp
    ui/widget/tab/managed_by.cpp
    ui/widget/tab/security.cpp
    ui/widget/tab/os.cpp
    ui/widget/tab/delegation.cpp
    ui/widget/tab/laps.cpp
    ui/widget/tab/error.cpp
    ui/widget/tab/group_policy.cpp
    ui/widget/tab/sites_link/common_widget.cpp
    ui/widget/tab/sites_link/part_widget.cpp
    ui/widget/tab/sites_link/widget.cpp
    ui/widget/tab/sites_link/general_tab.cpp
    ui/widget/tab/general_site.cpp
    utils.cpp

    main_window.cpp
    create_object_helper.cpp
    rename_object_helper.cpp

    fsmo/fsmo_dialog.cpp
    fsmo/fsmo_tab.cpp
    fsmo/fsmo_utils.cpp

    attribute_edits/attribute_edit.cpp
    attribute_edits/string_edit.cpp
    attribute_edits/string_other_edit.cpp
    attribute_edits/string_list_edit.cpp
    attribute_edits/string_large_edit.cpp
    attribute_edits/sam_name_edit.cpp
    attribute_edits/computer_sam_name_edit.cpp
    attribute_edits/country_edit.cpp
    attribute_edits/expiry_edit.cpp
    attribute_edits/expiry_widget.cpp
    attribute_edits/unlock_edit.cpp
    attribute_edits/group_scope_edit.cpp
    attribute_edits/group_type_edit.cpp
    attribute_edits/account_option_edit.cpp
    attribute_edits/password_edit.cpp
    attribute_edits/datetime_edit.cpp
    attribute_edits/gpoptions_edit.cpp
    attribute_edits/manager_edit.cpp
    attribute_edits/upn_edit.cpp
    attribute_edits/upn_suffix_combo.cpp
    attribute_edits/country_combo.cpp
    attribute_edits/manager_widget.cpp
    attribute_edits/delegation_edit.cpp
    attribute_edits/schedule_hours_edit.cpp
    attribute_edits/schedule_hours_dialog.cpp
    attribute_edits/logon_computers_edit.cpp
    attribute_edits/logon_computers_dialog.cpp
    attribute_edits/protect_deletion_edit.cpp
    attribute_edits/laps_expiry_edit.cpp
    attribute_edits/general_name_edit.cpp
    attribute_edits/dn_edit.cpp
    attribute_edits/pso_applied_edit.cpp
    attribute_edits/sites_link_edit.cpp
    attribute_edits/site_links_table_edit.cpp
    attribute_edits/subnets_list_edit.cpp

    attribute_edits/upn_multi_edit.cpp
    attribute_edits/account_option_multi_edit.cpp

    console_impls/object_impl/object_impl.cpp
    console_impls/object_impl/drag_n_drop.cpp
    console_impls/object_impl/console_object_operations.cpp
    console_impls/object_impl/site_dn_attrs_updater.cpp
    console_impls/object_impl/server_dn_attrs_updater.cpp
    console_impls/policy_impl.cpp
    console_impls/query_item_impl.cpp
    console_impls/query_folder_impl.cpp
    console_impls/policy_root_impl.cpp
    console_impls/find_object_impl.cpp
    console_impls/find_policy_impl.cpp
    console_impls/all_policies_folder_impl.cpp
    console_impls/policy_ou_impl.cpp
    console_impls/found_policy_impl.cpp
    console_impls/domain_info_impl.cpp

    permission_control_widgets/permissions_widget.cpp
    permission_control_widgets/creation_deletion_permissions_widget.cpp
    permission_control_widgets/read_write_permissions_widget.cpp
    permission_control_widgets/common_permissions_widget.cpp
    permission_control_widgets/extended_permissions_widget.cpp
    permission_control_widgets/delegation_permissions_widget.cpp
    permission_control_widgets/sddl_view_dialog.cpp

    admc.qrc
)

set(ADMC_SOURCES ${ADMC_SOURCES} ${LAPS_SOURCES})

set(ADMC_HEADERS
    ui/dialog/auth/base.h
)

add_executable(admc
    main.cpp
    ${ADMC_SOURCES}
    ${ADMC_HEADERS}
)
target_clangformat_setup(admc)

# NOTE: build admc as library to streamline building tests.
# Tests just link to this lib and use all of admc's code.
add_library(admctest SHARED
    ${ADMC_SOURCES}
    ${ADMC_HEADERS}
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
    COMMAND ${Qt6_LUPDATE_EXECUTABLE} -recursive ${CMAKE_CURRENT_SOURCE_DIR} -ts ${TS_FILES} -noobsolete
)

# Generate translation binaries (.qm) from sources (.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR})
qt6_add_translation(QM_FILES ${TS_FILES})

install(TARGETS admc DESTINATION ${CMAKE_INSTALL_BINDIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
if(NOT ADMC_BUILD_DEB)
    install(TARGETS admctest DESTINATION ${CMAKE_INSTALL_LIBDIR}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif(NOT ADMC_BUILD_DEB)
