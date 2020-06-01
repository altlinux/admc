cmake_minimum_required(VERSION 3.0)
project(gpgui VERSION 0.0.1)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Suppress warnings on FindDoxygen.cmake
cmake_policy(SET CMP0054 NEW)

add_subdirectory(libgptbackend)

if(SYSTEM_NAME STREQUAL "FreeBSD")
	message(STATUS "Using external libiconv for FreeBSD")
	include_directories(/usr/local/lib)
	link_directories(/usr/local/lib)
	set(Iconv_IS_BUILT_IN FALSE)
endif()

find_package(Qt5 COMPONENTS Core Widgets REQUIRED) # GUI
if(NOT Iconv_IS_BUILT_IN)
	find_package(Iconv) # For libgptbackend
endif()
find_package(Doxygen) # Code documentation

set(GPGUI_GUI_HEADER_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/gui/include
)

set(GPGUI_GUI_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/gui/MainWindow.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/gui/REG_DWORD_Dialog.cpp
)

set(GPGUI_HEADER_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(GPGUI_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Runner.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/gpgui.qrc
)

# Definitions for configuration file
set(GPGUI_APPLICATION_NAME "GPGUI")
set(GPGUI_APPLICATION_DISPLAY_NAME "GPGUI")
set(GPGUI_ORGANIZATION "BaseALT")
set(GPGUI_ORGANIZATION_DOMAIN "basealt.ru")

configure_file("config.h.in" "config.h")

add_definitions(${QT5_DEFINITIONS})
add_executable(gpgui
	${GPGUI_SOURCES}
	${GPGUI_GUI_SOURCES}
)

doxygen_add_docs(gpgui_doc
	${GPGUI_HEADER_DIRS}
	${GPGUI_GUI_HEADER_DIRS}
	COMMENT
	"Group Policy Editor GUI documentation"
)

if(SYSTEM_NAME STREQUAL "FreeBSD")
	target_link_libraries(gpgui
		Iconv::Iconv
	)
endif()

target_link_libraries(gpgui
	Qt5::Core
	Qt5::Widgets
	gptbackend
)

target_include_directories(gpgui
	PUBLIC
	${CMAKE_CURRENT_BINARY_DIR}
	${GPGUI_HEADER_DIRS}
	${GPGUI_GUI_HEADER_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/libgptbackend/preg/include
)

install(TARGETS gpgui)

