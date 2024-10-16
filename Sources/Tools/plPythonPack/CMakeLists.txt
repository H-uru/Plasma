set(plPythonPack_SOURCES
    main.cpp
    PythonInterface.cpp
)

set(plPythonPack_HEADERS
    PythonInterface.h
)

plasma_executable(plPythonPack TOOL
    FOLDER Tools
    SOURCES ${plPythonPack_SOURCES} ${plPythonPack_HEADERS}
)
target_link_libraries(
    plPythonPack
    PRIVATE
        CoreLib
        Python3::Python
)

source_group("Source Files" FILES ${plPythonPack_SOURCES})
source_group("Header Files" FILES ${plPythonPack_HEADERS})
