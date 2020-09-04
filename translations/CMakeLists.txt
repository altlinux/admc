find_package(Qt5 REQUIRED COMPONENTS
    LinguistTools
    )

set(TS_DIR ${PROJECT_SOURCE_DIR}/translations)

set(TS_FILES
    ${TS_DIR}/ru.ts
    ${TS_DIR}/en.ts
    )

set(QM_FILES
    ${TS_DIR}/ru.qm
    ${TS_DIR}/en.qm
    )

# Generate translation binaries (.qm) from sources (.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${TS_DIR})
qt5_add_translation(QM_FILES ${TS_FILES})

# NOTE: updating translations(.cpp -> .ts) is in a separate target so that it's not run every time source changes. When you do need to update translations, do `make update_translations`. *Generating* translations IS done every time (.ts -> .qm).

add_custom_target(update_translations
    DEPENDS ${TS_FILES}
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -recursive ${PROJECT_SOURCE_DIR} -ts ${TS_FILES}
    BYPRODUCTS ${PROJECT_SOURCE_DIR}/translations/translations.qrc
    )

add_custom_target(translations
    ALL
    DEPENDS ${QM_FILES})
