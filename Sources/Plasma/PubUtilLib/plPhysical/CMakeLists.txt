include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

set(plPhysical_SOURCES
    plCollisionDetector.cpp
    plPhysicalProxy.cpp
    plPhysicalSDLModifier.cpp
    plPhysicalSndGroup.cpp
    plPhysicsSoundMgr.cpp
    plPickingDetector.cpp
)

set(plPhysical_HEADERS
    plCollisionDetector.h
    plDetectorModifier.h
    plPhysicalCreatable.h
    plPhysicalProxy.h
    plPhysicalSDLModifier.h
    plPhysicalSndGroup.h
    plPhysicsSoundMgr.h
    plPickingDetector.h
    plSimDefs.h
)

add_library(plPhysical STATIC ${plPhysical_SOURCES} ${plPhysical_HEADERS})

source_group("Source Files" FILES ${plPhysical_SOURCES})
source_group("Header Files" FILES ${plPhysical_HEADERS})
