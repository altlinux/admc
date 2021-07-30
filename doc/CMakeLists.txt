
set(ADMC_HELP_WIKI ${CMAKE_CURRENT_SOURCE_DIR}/admc.wiki)
set(ADMC_HELP_PROJECT ${CMAKE_CURRENT_BINARY_DIR}/admc.qhp)
set(ADMC_COMPRESSED_HELP ${CMAKE_CURRENT_BINARY_DIR}/admc.qch)

find_package(Qt5 REQUIRED
    COMPONENTS
        Core
        Help
)

add_executable(generate-manual
    generate_manual.cpp
)

# NOTE: save generate-manual executable in the "doc" binary
# dir so that there's no error about unpackaged binaries in
# rpm build root
set_target_properties(generate-manual
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(generate-manual
    Qt5::Core
)

add_custom_command(
    OUTPUT ${ADMC_HELP_PROJECT}
    COMMAND generate-manual ${ADMC_HELP_WIKI} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS generate-manual ${ADMC_HELP_WIKI}
)

# Test target that only generates the html files
add_custom_target(doc_html
    ALL
    DEPENDS ${ADMC_HELP_PROJECT} generate-manual
)

# NOTE: This also depends on all the generated .html files
# but we don't know their names so won't include them
# here...
add_custom_command(
    OUTPUT ${ADMC_COMPRESSED_HELP}
    COMMAND Qt5::qhelpgenerator ${ADMC_HELP_PROJECT} -o ${ADMC_COMPRESSED_HELP}
    DEPENDS ${ADMC_HELP_PROJECT}
)

add_custom_target(doc
    ALL
    DEPENDS ${ADMC_COMPRESSED_HELP} generate-manual
)

install(
    FILES ${ADMC_COMPRESSED_HELP}
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

INSTALL(FILES ${PROJECT_SOURCE_DIR}/doc/admc.1
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1/
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

install(
    FILES admc.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)
