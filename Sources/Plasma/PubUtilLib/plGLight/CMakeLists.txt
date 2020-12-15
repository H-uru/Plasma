include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

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

add_library(plGLight STATIC ${plGLight_SOURCES} ${plGLight_HEADERS})

source_group("Source Files" FILES ${plGLight_SOURCES})
source_group("Header Files" FILES ${plGLight_HEADERS})
