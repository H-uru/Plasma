include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

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
