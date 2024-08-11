set(plAnimation_SOURCES
    plAGAnim.cpp
    plAGAnimInstance.cpp
    plAGApplicator.cpp
    plAGChannel.cpp
    plAGMasterMod.cpp
    plAGModifier.cpp
    plMatrixChannel.cpp
    plPointChannel.cpp
    plQuatChannel.cpp
    plScalarChannel.cpp
)

set(plAnimation_HEADERS
    plAGAnim.h
    plAGAnimInstance.h
    plAGApplicator.h
    plAGChannel.h
    plAGDefs.h
    plAGMasterMod.h
    plAGModifier.h
    plAnimationCreatable.h
    plMatrixChannel.h
    plPointChannel.h
    plQuatChannel.h
    plScalarChannel.h
)

plasma_library(plAnimation
    FOLDER PubUtilLib
    SOURCES ${plAnimation_SOURCES} ${plAnimation_HEADERS}
    UNITY_BUILD
    PRECOMPILED_HEADERS Pch.h
)
target_link_libraries(
    plAnimation
    PUBLIC
        CoreLib
        pnFactory
        pnModifier
        pnNetCommon
        plTransform
    PRIVATE
        pnMessage
        pnNucleusInc
        pnSceneObject
        plGLight
        plInterp
        plMessage
        plModifier
        plSDL
)

source_group("Source Files" FILES ${plAnimation_SOURCES})
source_group("Header Files" FILES ${plAnimation_HEADERS})
