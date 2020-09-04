find_package(Qt5 REQUIRED COMPONENTS
    LinguistTools
    )

set(TS_DIR ${PROJECT_SOURCE_DIR}/translations)

set(TS_FILES
    ${TS_DIR}/ru.ts
    ${TS_DIR}/en.ts
    )

# Do `make update_translations` to update translation sources (.ts)
# NOTE: this is in a custom target so that translation sources aren't rebuilt everytime app source changes
add_custom_target(update_translations
    DEPENDS ${TS_FILES}
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -recursive ${PROJECT_SOURCE_DIR} -ts ${TS_FILES}
    )

# Generate translation binaries (.qm) from sources (.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${TS_DIR})
qt5_add_translation(QM_FILES ${TS_FILES})
