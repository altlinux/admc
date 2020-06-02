cmake_minimum_required(VERSION 3.0)
project(gptbackend VERSION 0.0.1)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

# Suppress warnings on FindDoxygen.cmake
cmake_policy(SET CMP0054 NEW)

if(SYSTEM_NAME STREQUAL "FreeBSD")
	link_directories(/usr/local/lib)
	set(Iconv_IS_BUILT_IN FALSE)
	find_package(Iconv) # PReg key name charset conversion
endif()

#find_package(Catch2 REQUIRED) # Unit tests
find_package(Doxygen) # Documentation
#find_package(PkgConfig)
# There is XML registry format produced by python-samba. This library is
# a dependency for producing XML files of this format.
#pkg_check_modules(PUGIXML REQUIRED IMPORTED_TARGET pugixml)
#pkg_check_modules(CATCH2  REQUIRED IMPORTED_TARGET catch)

set(PREG_PARSER_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/preg/iconv_wrapper.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/preg/preg_data.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/preg/preg_parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/preg/preg_writer.cpp
)

set(GPTBACKEND_HEADER_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/preg/include
)

set(GPTBACKEND_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

set(TEST_GPTBACKEND_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test/test_preg_data.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test/test_preg_writer.cpp
)

# Definitions for configuration file
configure_file("config.h.in" "config.h")

#add_executable(gptbackend_test
#	${TEST_GPTBACKEND_SOURCES}
#	${PREG_PARSER_SOURCES}
#)

add_library(gptbackend SHARED
	${GPTBACKEND_SOURCES}
	${PREG_PARSER_SOURCES}
)

doxygen_add_docs(gptbackend_doc
	${GPTBACKEND_HEADER_DIRS}
	COMMENT
	"libgptbackend documentation"
)

#target_include_directories(gptbackend_test
#	PUBLIC
#	${CMAKE_CURRENT_BINARY_DIR}
#	${GPTBACKEND_HEADER_DIRS}
#)

target_include_directories(gptbackend
	PUBLIC
	${CMAKE_CURRENT_BINARY_DIR}
	${GPTBACKEND_HEADER_DIRS}
)

include(CTest)
#include(ParseAndAddCatchTests)
#ParseAndAddCatchTests(gptbackend_test)

#target_link_libraries(gptbackend_test
	#PkgConfig::CATCH2
	#Catch2::Catch2
#	gptbackend
	#Iconv::Iconv
#)

target_link_libraries(gptbackend PUBLIC
	#Iconv::Iconv
)

install(TARGETS gptbackend)
#install(TARGETS gptbackend_test RUNTIME DESTINATION ${CMAKE_BINARY_DIR})

