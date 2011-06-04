include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

set(pfConditional_SOURCES
    plActivatorConditionalObject.cpp
    plANDConditionalObject.cpp
    plAnimationEventConditionalObject.cpp
    plControlEventConditionalObject.cpp
    plFacingConditionalObject.cpp
    plKeyPressConditionalObject.cpp
    plLocalPlayerInBoxConditionalObject.cpp
    plLocalPlayerIntersectPlaneConditionalObject.cpp
    plObjectInBoxConditionalObject.cpp
    plObjectIntersectPlaneConditionalObject.cpp
    plORConditionalObject.cpp
    plPickedConditionalObject.cpp
)

set(pfConditional_HEADERS
    plActivatorConditionalObject.h
    plANDConditionalObject.h
    plAnimationEventConditionalObject.h
    plConditionalObjectCreatable.h
    plControlEventConditionalObject.h
    plFacingConditionalObject.h
    plKeyPressConditionalObject.h
    plLocalPlayerInBoxConditionalObject.h
    plLocalPlayerIntersectPlaneConditionalObject.h
    plObjectInBoxConditionalObject.h
    plObjectIntersectPlaneConditionalObject.h
    plORConditionalObject.h
    plPickedConditionalObject.h
)

add_library(pfConditional ${pfConditional_SOURCES} ${pfConditional_HEADERS})

source_group("Source Files" FILES ${pfConditional_SOURCES})
source_group("Header Files" FILES ${pfConditional_HEADERS})
