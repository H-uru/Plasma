set(plPhysical_SOURCES
    plCollisionDetector.cpp
    plDetectorModifier.cpp
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

plasma_library(plPhysical
    FOLDER PubUtilLib
    SOURCES ${plPhysical_SOURCES} ${plPhysical_HEADERS}
)
target_link_libraries(
    plPhysical
    PUBLIC
        CoreLib
        pnKeyedObject
        pnNetCommon
        plDrawable
        plModifier
    PRIVATE
        pnMessage
        pnNucleusInc
        pnSceneObject
        plAvatar
        plMessage
        plPhysX
        plSDL
        plStatusLog
        plSurface
    INTERFACE
        pnFactory
)

target_include_directories(plPhysical PRIVATE "${PLASMA_SOURCE_ROOT}/FeatureLib")

source_group("Source Files" FILES ${plPhysical_SOURCES})
source_group("Header Files" FILES ${plPhysical_HEADERS})
