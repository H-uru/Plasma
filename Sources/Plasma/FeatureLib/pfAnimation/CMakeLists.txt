include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

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
