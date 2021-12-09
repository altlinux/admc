cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# NOTE: need largefile options to prevent problems with
# smbclient on 32bit systems
add_compile_options(-Wall -Wextra -Werror=shadow -Werror=switch -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)

# You can get version from spec by first finding Versions keyword.
# After that you can use awk to split line by : and then select second part of it.
# Finally you want to clear all the spaces around version.
# OUTPUT_VARIABLE argument of execute_process allows you to capture output of the command.
execute_process(
    COMMAND
    bash -c "grep Version: .gear/admc.spec | awk -F\":\" '{ print $2 }' | tr -d [:space:]"
    OUTPUT_VARIABLE GEAR_VERSION
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# To ensure that we have indeed captured the project version we use message to provide us with console output.
message(STATUS "Project version is: " ${GEAR_VERSION})

project(admc VERSION ${GEAR_VERSION})

message(STATUS "Project version is: " ${VERSION})

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}/cmake
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})

enable_testing()

include(GNUInstallDirs)

option(ADMC_BUILD_DEB "Build the deb package of ADMC." OFF)

add_subdirectory(src)
if(NOT ADMC_BUILD_DEB)
    add_subdirectory(tests)
endif(NOT ADMC_BUILD_DEB)
add_subdirectory(doc)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.txt
    ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

if (ADMC_BUILD_DEB)
    SET(CPACK_GENERATOR "DEB")
    SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kees van Vloten")
    SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Samba domain management console.")
    SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/altlinux/admc")
    set(CPACK_STRIP_FILES "${CMAKE_INSTALL_LIBDIR}/libadldap.so;${CMAKE_INSTALL_BINDIR}/admc")
    INCLUDE(CPack)
endif(ADMC_BUILD_DEB)
