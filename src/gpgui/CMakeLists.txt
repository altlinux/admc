
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(libgptbackend)

if(SYSTEM_NAME STREQUAL "FreeBSD")
	message(STATUS "Using external libiconv for FreeBSD")
	include_directories(/usr/local/lib)
	link_directories(/usr/local/lib)
	set(Iconv_IS_BUILT_IN FALSE)
endif()

find_package(Qt5 REQUIRED
    COMPONENTS
        Core
        Widgets
        Xml
)
find_package(Smbclient REQUIRED)
if(NOT Iconv_IS_BUILT_IN)
	find_package(Iconv) # For libgptbackend
endif()

set(GPGUI_APPLICATION_NAME "GPGUI")
set(GPGUI_APPLICATION_DISPLAY_NAME "GPGUI")
set(GPGUI_ORGANIZATION "BaseALT")
set(GPGUI_ORGANIZATION_DOMAIN "basealt.ru")
configure_file("config.h.in" "config.h")

add_executable(gpgui
    main.cpp
    gui/MainWindow.cpp
    gui/browse_widget.cpp
    gui/pol_editor.cpp
    gui/file.cpp
    
    gui/xml/xml_editor.cpp
    gui/xml/xml_attribute.cpp
    gui/xml/xml_edit.cpp
    gui/xml/xml_string_edit.cpp
    gui/xml/xml_bool_edit.cpp
    gui/xml/xml_ubyte_edit.cpp

    gpgui.qrc
)

if(SYSTEM_NAME STREQUAL "FreeBSD")
	target_link_libraries(gpgui
		Iconv::Iconv
	)
endif()

target_link_libraries(gpgui
    PRIVATE
    	Qt5::Core
        Qt5::Widgets
    	Qt5::Xml
    	gptbackend
        Smbclient::Smbclient
)

target_include_directories(gpgui
	PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/libgptbackend/preg
        ${CMAKE_CURRENT_SOURCE_DIR}/gui/
        ${CMAKE_CURRENT_SOURCE_DIR}/gui/xml
)

install(TARGETS gpgui)
