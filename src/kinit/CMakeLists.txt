find_package(Qt5 REQUIRED
    COMPONENTS
    Core
    Widgets
    Network
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_library(kinit SHARED
    kinitdialog.cpp
    krb5ticketwatcher.cpp
    v5.cpp
    pw_dialog.cpp
    pw_change_dialog.cpp
)

target_include_directories(kinit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(kinit
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
)

if(NOT ADMC_BUILD_DEB)
    target_compile_definitions(kinit PRIVATE SMB_HAS_NEW_NDR_PULL_STEAL_SWITCH=1)
endif(NOT ADMC_BUILD_DEB)
