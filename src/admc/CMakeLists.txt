find_package(Qt5 REQUIRED
    COMPONENTS
        Core
        Widgets
        Help
        LinguistTools
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

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
    properties_dialog.cpp
    main_window.cpp
    status.cpp
    select_object_dialog.cpp
    console_actions.cpp
    password_dialog.cpp
    find_results.cpp
    filter_dialog.cpp
    find_widget.cpp
    tab_widget.cpp
    central_widget.cpp
    search_thread.cpp
    help_browser.cpp
    globals.cpp
    utils.cpp
    settings.cpp
    policy_results_widget.cpp
    gplink.cpp
    
    console_types/console_object.cpp
    console_types/console_policy.cpp
    console_types/console_query.cpp

    move_object_dialog.cpp
    rename_object_dialog.cpp
    rename_policy_dialog.cpp
    create_object_dialog.cpp
    find_object_dialog.cpp
    find_select_object_dialog.cpp
    select_container_dialog.cpp
    about_dialog.cpp
    manual_dialog.cpp
    select_policy_dialog.cpp
    create_policy_dialog.cpp
    create_query_dialog.cpp
    create_query_folder_dialog.cpp
    edit_query_folder_dialog.cpp
    move_query_dialog.cpp
    object_multi_properties_dialog.cpp

    filter_widget/filter_widget.cpp
    filter_widget/filter_widget_simple_tab.cpp
    filter_widget/filter_widget_normal_tab.cpp
    filter_widget/filter_widget_advanced_tab.cpp
    filter_widget/select_classes_widget.cpp
    filter_widget/filter_builder.cpp
    filter_widget/search_base_widget.cpp
    
    tabs/properties_tab.cpp
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
    tabs/managed_by_tab.cpp

    editors/attribute_editor.cpp
    editors/multi_editor.cpp
    editors/string_editor.cpp
    editors/octet_editor.cpp
    editors/bool_editor.cpp
    editors/datetime_editor.cpp

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
    edits/upn_edit.cpp

    console_widget/console_widget.cpp
    console_widget/scope_model.cpp
    console_widget/customize_columns_dialog.cpp
    console_widget/results_description.cpp
    console_widget/results_view.cpp
    console_widget/console_drag_model.cpp

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

install(TARGETS admc)
install(TARGETS admctest)
