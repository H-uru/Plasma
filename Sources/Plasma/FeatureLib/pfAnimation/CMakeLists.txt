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

target_link_libraries(pfAnimation
    PUBLIC
        CoreLib
        pnModifier
        plModifier
)

source_group("Source Files" FILES ${pfAnimation_SOURCES})
source_group("Header Files" FILES ${pfAnimation_HEADERS})
