set(plPhysX_SOURCES
    plGenericPhysical.cpp
    plKickableLog.cpp
    plLOSDispatch.cpp
    plPXConvert.cpp
    plPXCooking.cpp
    plPXLOSDispatch.cpp
    plPXPhysical.cpp
    plPXPhysicalControllerCore.cpp
    plPXSimulation.cpp
    plPXSubWorld.cpp
    plSimulationMgr.cpp
)

set(plPhysX_HEADERS
    plKickableLog.h
    plLOSDispatch.h
    plPhysXAPI.h
    plPhysXCreatable.h
    plPXConvert.h
    plPXCooking.h
    plPXPhysical.h
    plPXPhysicalControllerCore.h
    plPXSimDefs.h
    plPXSimulation.h
    plPXSubWorld.h
    plSimulationMgr.h
)

plasma_library(plPhysX
    SOURCES ${plPhysX_SOURCES} ${plPhysX_HEADERS}
    UNITY_BUILD
    PRECOMPILED_HEADERS Pch.h
)

target_link_libraries(plPhysX
    PUBLIC
        CoreLib
        pnKeyedObject
        pnSceneObject
        pnNucleusInc
        plAvatar
        plPhysical
        plStatusLog
    PRIVATE
        pnMessage
        pnNetCommon
        pnSceneObject
        plDrawable
        plMessage
        plModifier
        plPhysical
        plSurface
        PhysX::PhysX
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${plPhysX_SOURCES})
source_group("Header Files" FILES ${plPhysX_HEADERS})
