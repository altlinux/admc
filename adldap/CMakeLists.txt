cmake_minimum_required(VERSION 3.0)
project(adldap VERSION 0.1.0)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_FLAGS "-Wall -Wshadow")

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules
)

find_package(OpenLDAP REQUIRED)
find_package(Doxygen) # Code documentation


set(ADLDAP_HEADER_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(ADLDAP_C_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/active_directory.c
)

set(ADLDAP_CPP_SOURCES
	${ADLDAP_C_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/ad_connection.cpp
)

add_library(adldap SHARED ${ADLDAP_C_SOURCES})
target_include_directories(adldap PUBLIC ${ADLDAP_HEADER_DIRS})
target_link_libraries(adldap ${OPENLDAP_LIBRARIES})

add_library(adldap++ SHARED ${ADLDAP_CPP_SOURCES})
target_include_directories(adldap++ PUBLIC ${ADLDAP_HEADER_DIRS})
target_link_libraries(adldap++ ${OPENLDAP_LIBRARIES})

install(TARGETS adldap)
install(TARGETS adldap++)

