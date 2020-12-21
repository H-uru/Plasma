find_package(PhysX REQUIRED)
set_package_properties(PhysX PROPERTIES
    URL "https://developer.nvidia.com/physx-sdk"
    DESCRIPTION "Scalable multi-platform physics solution"
    TYPE REQUIRED
)


include_directories(${PHYSX_INCLUDE_DIRS})

set(plPhysX_SOURCES
    plGenericPhysical.cpp
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

add_library(plPhysX STATIC ${plPhysX_SOURCES} ${plPhysX_HEADERS})

target_link_libraries(plPhysX
    PUBLIC
        ${PHYSX_LIBRARIES}
        CoreLib
        plPhysical
)

source_group("Source Files" FILES ${plPhysX_SOURCES})
source_group("Header Files" FILES ${plPhysX_HEADERS})
