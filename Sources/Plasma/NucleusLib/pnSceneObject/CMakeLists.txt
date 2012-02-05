include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../NucleusLib/inc")
include_directories("../../PubUtilLib")

set(pnSceneObject_HEADERS
    plAudioInterface.h
    plCoordinateInterface.h
    plDrawInterface.h
    plObjInterface.h
    plSceneObject.h
    plSimulationInterface.h
    pnSceneObjectCreatable.h
)

set(pnSceneObject_SOURCES
    plAudioInterface.cpp
    plCoordinateInterface.cpp
    plDrawInterface.cpp
    plObjInterface.cpp
    plSceneObject.cpp
    plSimulationInterface.cpp
)

add_library(pnSceneObject STATIC ${pnSceneObject_HEADERS} ${pnSceneObject_SOURCES})
target_link_libraries(pnSceneObject CoreLib pnKeyedObject pnNetCommon)

source_group("Header Files" FILES ${pnSceneObject_HEADERS})
source_group("Source Files" FILES ${pnSceneObject_SOURCES})
