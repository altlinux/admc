cmake_minimum_required(VERSION 3.0)

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

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}/cmake
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})

enable_testing()

add_subdirectory(translations)
add_subdirectory(src)
add_subdirectory(tests)
