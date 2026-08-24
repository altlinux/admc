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
        ui/attribute_edit/laps_encrypted_attribute_edit.cpp
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
    core/console/object/drag_n_drop.cpp
    core/console/object/operations.cpp
    core/console/object/site_dn_attrs_updater.cpp
    core/console/object/server_dn_attrs_updater.cpp
    core/fsmo.cpp
    core/globals.cpp
    core/line_edit_utils.cpp
    core/managers/country_manager.cpp
    core/managers/gplink_manager.cpp
    core/managers/icon_manager.cpp
    core/search_thread.cpp
    core/settings.cpp
    core/utils.cpp
    ui/console/all_policies_folder_impl.cpp
    ui/console/domain_info_impl.cpp
    ui/console/find_object_impl.cpp
    ui/console/find_policy_impl.cpp
    ui/console/found_policy_impl.cpp
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
    ui/dialog/fsmo.cpp
    ui/dialog/main_window_connection_error.cpp
    ui/dialog/password.cpp
    ui/dialog/properties.cpp
    ui/dialog/properties_multi.cpp
    ui/dialog/properties_warning.cpp
    ui/dialog/rename/group.cpp
    ui/dialog/rename/other.cpp
    ui/dialog/rename/policy.cpp
    ui/dialog/rename/user.cpp
    ui/dialog/sddl_view.cpp
    ui/dialog/security_sort_warning.cpp
    ui/dialog/select/object.cpp
    ui/dialog/select/object_match.cpp
    ui/dialog/select/object_advanced.cpp
    ui/dialog/select/container.cpp
    ui/dialog/select/policy.cpp
    ui/dialog/select/well_known_trustee.cpp
    ui/message_box.cpp
    ui/status.cpp
    ui/utils.cpp
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
    ui/widget/fsmo/fsmo_tab.cpp
    ui/widget/fsmo/fsmo_utils.cpp
    ui/widget/multi_tab/general_user.cpp
    ui/widget/multi_tab/general_other.cpp
    ui/widget/multi_tab/account.cpp
    ui/widget/multi_tab/address.cpp
    ui/widget/multi_tab/profile.cpp
    ui/widget/multi_tab/organization.cpp
    ui/widget/permission_control/common_permissions.cpp
    ui/widget/permission_control/creation_deletion_permissions.cpp
    ui/widget/permission_control/delegation_permissions.cpp
    ui/widget/permission_control/extended_permissions.cpp
    ui/widget/permission_control/permissions.cpp
    ui/widget/permission_control/read_write_permissions.cpp
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

    main_window.cpp
    ui/create_object_helper.cpp
    ui/rename_object_helper.cpp

    ui/attribute_edit/attribute_edit.cpp
    ui/attribute_edit/string_edit.cpp
    ui/attribute_edit/string_other_edit.cpp
    ui/attribute_edit/string_list_edit.cpp
    ui/attribute_edit/string_large_edit.cpp
    ui/attribute_edit/sam_name_edit.cpp
    ui/attribute_edit/computer_sam_name_edit.cpp
    ui/attribute_edit/country_edit.cpp
    ui/attribute_edit/expiry_edit.cpp
    ui/attribute_edit/expiry_widget.cpp
    ui/attribute_edit/unlock_edit.cpp
    ui/attribute_edit/group_scope_edit.cpp
    ui/attribute_edit/group_type_edit.cpp
    ui/attribute_edit/account_option_edit.cpp
    ui/attribute_edit/password_edit.cpp
    ui/attribute_edit/datetime_edit.cpp
    ui/attribute_edit/gpoptions_edit.cpp
    ui/attribute_edit/manager_edit.cpp
    ui/attribute_edit/upn_edit.cpp
    ui/attribute_edit/upn_suffix_combo.cpp
    ui/attribute_edit/country_combo.cpp
    ui/attribute_edit/manager_widget.cpp
    ui/attribute_edit/delegation_edit.cpp
    ui/attribute_edit/schedule_hours_edit.cpp
    ui/attribute_edit/schedule_hours_dialog.cpp
    ui/attribute_edit/logon_computers_edit.cpp
    ui/attribute_edit/logon_computers_dialog.cpp
    ui/attribute_edit/protect_deletion_edit.cpp
    ui/attribute_edit/laps_expiry_edit.cpp
    ui/attribute_edit/general_name_edit.cpp
    ui/attribute_edit/dn_edit.cpp
    ui/attribute_edit/pso_applied_edit.cpp
    ui/attribute_edit/sites_link_edit.cpp
    ui/attribute_edit/site_links_table_edit.cpp
    ui/attribute_edit/subnets_list_edit.cpp

    ui/attribute_edit/upn_multi_edit.cpp
    ui/attribute_edit/account_option_multi_edit.cpp

    console_impls/object_impl/object_impl.cpp
    console_impls/object_impl/console_object_operations.cpp
    console_impls/policy_impl.cpp
    console_impls/query_item_impl.cpp
    console_impls/query_folder_impl.cpp
    console_impls/policy_root_impl.cpp
    console_impls/policy_ou_impl.cpp

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
