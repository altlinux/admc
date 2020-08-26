cmake_minimum_required(VERSION 3.0)
project(admc VERSION 0.1.0)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_FLAGS "-Wall -Wshadow -Werror=switch")

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

find_package(Qt5 COMPONENTS Core Widgets LinguistTools REQUIRED) # GUI
find_package(Doxygen) # Code documentation

# Definitions for configuration file
set(ADMC_APPLICATION_NAME "ADMC")
set(ADMC_APPLICATION_DISPLAY_NAME "ADMC")
set(ADMC_ORGANIZATION "BaseALT")
set(ADMC_ORGANIZATION_DOMAIN "basealt.ru")

configure_file("config.h.in" "config.h")

add_subdirectory(adldap)
add_subdirectory(gpgui)
# add_subdirectory(translations)

set(ADMC_HEADER_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/adldap/include
)

set(ADMC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ad_interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/details_widget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/details_tab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/attributes_tab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/containers_widget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/contents_widget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main_window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/status.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/members_tab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/object_model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/move_dialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/object_context_menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/confirmation_dialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/advanced_view_proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dn_column_proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/login_dialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/password_dialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/menubar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/account_tab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/general_tab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/address_tab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/object_tab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rename_dialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/create_dialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/attribute_display_strings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/attribute_edit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/country_edit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/expiry_edit.cpp
)

set(TS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/translations)
set(TS_FILES
    ${TS_DIR}/ru.ts
    ${TS_DIR}/en.ts
)

# `make translations` to generate translation sources (.ts)
# NOTE: put this in a custom target so that translations aren't rebuilt on every source change
add_custom_target(translations
    DEPENDS ${TS_FILES}
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR} -ts ${TS_FILES}
)

# Generate translation binaries (.qm) from sources (.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${TS_DIR})
qt5_add_translation(QM_FILES ${TS_FILES})

add_definitions(${QT5_DEFINITIONS})
add_executable(admc
    ${ADMC_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/admc.qrc
    ${TS_DIR}/translations.qrc
)

doxygen_add_docs(admc_doc
    ${ADMC_HEADER_DIRS}
    COMMENT
    "Group Policy Editor GUI documentation"
)


target_link_libraries(admc
    Qt5::Core
    Qt5::Widgets
    adldap++
)
target_include_directories(admc
    PUBLIC
    ${ADMC_HEADER_DIRS}
)

install(TARGETS admc)

