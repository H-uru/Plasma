set(plGeneratePythonStubs_SOURCES
    plAllCreatables.cpp
    main.cpp
)

set(plGeneratePythonStubs_HEADERS
)

plasma_executable(
    plGeneratePythonStubs TOOL
    FOLDER Tools
    SOURCES
        ${plGeneratePythonStubs_SOURCES}
        ${plGeneratePythonStubs_HEADERS}
)

target_link_libraries(
    plGeneratePythonStubs
    PRIVATE
        CoreLib
        pnNucleusInc
        plPubUtilInc
        pfFeatureInc
        pfPython

        Python3::Python
)
