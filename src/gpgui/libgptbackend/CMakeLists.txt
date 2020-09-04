if(SYSTEM_NAME STREQUAL "FreeBSD")
	link_directories(/usr/local/lib)
	set(Iconv_IS_BUILT_IN FALSE)
	find_package(Iconv) # PReg key name charset conversion
endif()

add_library(gptbackend SHARED
    main.cpp
    preg/iconv_wrapper.cpp
    preg/preg_data.cpp
    preg/preg_parser.cpp
    preg/preg_writer.cpp
)

target_include_directories(gptbackend
	PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS gptbackend)
