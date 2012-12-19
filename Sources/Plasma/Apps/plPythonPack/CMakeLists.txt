include_directories(../../CoreLib)
include_directories(../../PubUtilLib)

include_directories(${PYTHON_INCLUDE_DIR})

set(plPythonPack_SOURCES
    main.cpp
    PythonInterface.cpp
)

set(plPythonPack_HEADERS
    PythonInterface.h
)

add_executable(plPythonPack ${plPythonPack_SOURCES} ${plPythonPack_HEADERS})
target_link_libraries(plPythonPack CoreLib plFile)

if(PYTHON_DEBUG_LIBRARY)
    target_link_libraries(plPythonPack debug ${PYTHON_DEBUG_LIBRARY})
    target_link_libraries(plPythonPack optimized ${PYTHON_LIBRARY})
else()
    target_link_libraries(plPythonPack ${PYTHON_LIBRARY})
endif()

if(USE_VLD)
    target_link_libraries(plPythonPack ${VLD_LIBRARY})
endif()

source_group("Source Files" FILES ${plPythonPack_SOURCES})
source_group("Header Files" FILES ${plPythonPack_HEADERS})
