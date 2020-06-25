cmake_minimum_required(VERSION 3.0)
project(adtool VERSION 0.1.0)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_FLAGS "-Wall -Wshadow")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Suppress warnings on FindDoxygen.cmake
cmake_policy(SET CMP0054 NEW)

if(SYSTEM_NAME STREQUAL "FreeBSD")
    set(Iconv_IS_BUILT_IN FALSE)
endif()

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules
)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED) # GUI
find_package(Doxygen) # Code documentation

# Definitions for configuration file
set(ADTOOL_APPLICATION_NAME "ADTOOL")
set(ADTOOL_APPLICATION_DISPLAY_NAME "ADTOOL")
set(ADTOOL_ORGANIZATION "BaseALT")
set(ADTOOL_ORGANIZATION_DOMAIN "basealt.ru")

configure_file("config.h.in" "config.h")

add_subdirectory(adldap)
add_subdirectory(gpgui)

set(ADTOOL_HEADER_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/adldap/include
)

set(ADTOOL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/admc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Runner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ad_interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/details_widget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/attributes_widget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/containers_widget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/contents_widget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main_window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/status.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/members_widget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/entry_model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/entry_context_menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/confirmation_dialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/advanced_view_proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dn_column_proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp
)

add_definitions(${QT5_DEFINITIONS})
add_executable(adtool
    ${ADTOOL_SOURCES}
)

doxygen_add_docs(adtool_doc
    ${ADTOOL_HEADER_DIRS}
    COMMENT
    "Group Policy Editor GUI documentation"
)


target_link_libraries(adtool
    Qt5::Core
    Qt5::Widgets
    adldap++
)
target_include_directories(adtool
    PUBLIC
    ${ADTOOL_HEADER_DIRS}
)

install(TARGETS adtool)

