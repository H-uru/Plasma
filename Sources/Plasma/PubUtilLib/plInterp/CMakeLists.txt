set(plInterp_SOURCES
    hsInterp.cpp
    hsKeys.cpp
    plAnimPath.cpp
    plAnimTimeConvert.cpp
    plATCEaseCurves.cpp
    plController.cpp
    plModulator.cpp
)

set(plInterp_HEADERS
    hsInterp.h
    hsKeys.h
    hsTimedValue.h
    plAnimEaseTypes.h
    plAnimPath.h
    plAnimTimeConvert.h
    plController.h
    plInterpCreatable.h
    plModulator.h
)

plasma_library(plInterp
    FOLDER PubUtilLib
    SOURCES ${plInterp_SOURCES} ${plInterp_HEADERS}
    PRECOMPILED_HEADERS Pch.h
)

target_link_libraries(plInterp
    PUBLIC
        CoreLib
        pnFactory
        pnNetCommon
        plIntersect
        plTransform
    PRIVATE
        pnMessage
        pnNetCommon
        pnNucleusInc
        plIntersect
        plMessage
)

source_group("Source Files" FILES ${plInterp_SOURCES})
source_group("Header Files" FILES ${plInterp_HEADERS})
