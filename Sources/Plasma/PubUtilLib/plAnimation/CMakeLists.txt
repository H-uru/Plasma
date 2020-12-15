include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

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

add_library(plAnimation STATIC ${plAnimation_SOURCES} ${plAnimation_HEADERS})

target_link_libraries(plAnimation pnModifier)
target_link_libraries(plAnimation plInterp)
target_link_libraries(plAnimation plSDL)
target_link_libraries(plAnimation plTransform)

source_group("Source Files" FILES ${plAnimation_SOURCES})
source_group("Header Files" FILES ${plAnimation_HEADERS})
