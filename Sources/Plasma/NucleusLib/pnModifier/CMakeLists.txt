set(pnModifier_HEADERS
    plConditionalObject.h
    plLogicModBase.h
    plModifier.h
    plMultiModifier.h
    plSingleModifier.h
    pnModifierCreatable.h
)

set(pnModifier_SOURCES
    plConditionalObject.cpp
    plLogicModBase.cpp
    plModifier.cpp
    plMultiModifier.cpp
    plSingleModifier.cpp
)

plasma_library(pnModifier
    SOURCES ${pnModifier_HEADERS} ${pnModifier_SOURCES}
    PRECOMPILED_HEADERS Pch.h
)
target_link_libraries(
    pnModifier
    PUBLIC
        CoreLib
        pnFactory
        pnKeyedObject
        pnNetCommon
    PRIVATE
        pnMessage
        plNetMessage
        pnNucleusInc
        pnSceneObject
)

target_include_directories(pnModifier PRIVATE "${PLASMA_SOURCE_ROOT}/PubUtilLib")

source_group("Header Files" FILES ${pnModifier_HEADERS})
source_group("Source Files" FILES ${pnModifier_SOURCES})
