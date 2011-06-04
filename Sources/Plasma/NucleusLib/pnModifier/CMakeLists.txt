include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

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

add_library(pnModifier STATIC ${pnModifier_HEADERS} ${pnModifier_SOURCES})

source_group("Header Files" FILES ${pnModifier_HEADERS})
source_group("Source Files" FILES ${pnModifier_SOURCES})
