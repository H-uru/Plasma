set(plGLight_SOURCES
    plDirectShadowMaster.cpp
    plLightInfo.cpp
    plLightProxy.cpp
    plLightSpace.cpp
    plPerspDirSlave.cpp
    plPointShadowMaster.cpp
    plShadowCaster.cpp
    plShadowMaster.cpp
    plShadowSlave.cpp
)

set(plGLight_HEADERS
    plDirectShadowMaster.h
    plGLightCreatable.h
    plLightInfo.h
    plLightKonstants.h
    plLightProxy.h
    plLightSpace.h
    plPerspDirSlave.h
    plPointShadowMaster.h
    plShadowCaster.h
    plShadowMaster.h
    plShadowSlave.h
)

plasma_library(plGLight
    FOLDER PubUtilLib
    SOURCES ${plGLight_SOURCES} ${plGLight_HEADERS}
)
target_link_libraries(plGLight
    PUBLIC
        CoreLib
        pnKeyedObject
        pnModifier
        pnSceneObject
        plDrawable
        plScene
    PRIVATE
        pnMessage
        pnNucleusInc
        plIntersect
        plMessage
        plPipeline
        plSurface
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${plGLight_SOURCES})
source_group("Header Files" FILES ${plGLight_HEADERS})
