cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wshadow -Werror=switch)

project(admc VERSION 0.6.4)

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
