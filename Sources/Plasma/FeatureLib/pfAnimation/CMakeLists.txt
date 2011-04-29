include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../NucleusLib/inc")
include_directories("../../PubUtilLib")

set(pfAnimation_SOURCES
    pfObjectFlocker.cpp
    plAnimDebugList.cpp
    plBlower.cpp
    plFilterCoordInterface.cpp
    plFollowMod.cpp
    plLightModifier.cpp
    plLineFollowMod.cpp
    plRandomCommandMod.cpp
    plStereizer.cpp
    plViewFaceModifier.cpp
)

set(pfAnimation_HEADERS
    pfAnimationCreatable.h
    pfObjectFlocker.h
    plAnimDebugList.h
    plBlower.h
    plFilterCoordInterface.h
    plFollowMod.h
    plLightModifier.h
    plLineFollowMod.h
    plRandomCommandMod.h
    plStereizer.h
    plViewFaceModifier.h
)

add_library(pfAnimation STATIC ${pfAnimation_SOURCES} ${pfAnimation_HEADERS})

source_group("Source Files" FILES ${pfAnimation_SOURCES})
source_group("Header Files" FILES ${pfAnimation_HEADERS})
