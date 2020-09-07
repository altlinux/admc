option(ENABLE_TESTS "Enable unit tests for libadldap" ON)

find_package(Ldap REQUIRED)

add_library(adldap SHARED
    active_directory.c
)

target_include_directories(adldap
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(adldap
    PUBLIC
        Ldap::Ldap
)

install(TARGETS adldap)

if(ENABLE_TESTS)
	find_package(CMocka CONFIG REQUIRED)
	include(AddCMockaTest)
	include(AddMockedTest)
	enable_testing()
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()
