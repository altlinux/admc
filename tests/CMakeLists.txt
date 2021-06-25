
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
    admc_test_select_base_widget
    admc_test_filter_widget
    admc_test_security_tab
    admc_test_attributes_tab
    admc_test_members_tab
    admc_test_member_of_tab
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
