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

plasma_library(pnSceneObject
    FOLDER NucleusLib
    SOURCES ${pnSceneObject_HEADERS} ${pnSceneObject_SOURCES}
)
target_link_libraries(
    pnSceneObject
    PUBLIC
        CoreLib
        pnFactory
        pnKeyedObject
        pnNetCommon
    PRIVATE
        pnMessage
        pnNetCommon
        pnNucleusInc
)

source_group("Header Files" FILES ${pnSceneObject_HEADERS})
source_group("Source Files" FILES ${pnSceneObject_SOURCES})
