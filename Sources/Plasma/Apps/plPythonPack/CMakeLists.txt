set(plPythonPack_SOURCES
    main.cpp
    PythonInterface.cpp
)

set(plPythonPack_HEADERS
    PythonInterface.h
)

add_executable(plPythonPack ${plPythonPack_SOURCES} ${plPythonPack_HEADERS})
target_link_libraries(
    plPythonPack
    PUBLIC
        CoreLib
        Python3::Python

    PRIVATE
        plFile
        string_theory
)

if(USE_VLD)
    target_link_libraries(plPythonPack PRIVATE ${VLD_LIBRARY})
endif()

install(
    TARGETS plPythonPack
    DESTINATION tools_cli
)

source_group("Source Files" FILES ${plPythonPack_SOURCES})
source_group("Header Files" FILES ${plPythonPack_HEADERS})
