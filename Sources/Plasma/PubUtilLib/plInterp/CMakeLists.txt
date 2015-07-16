include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

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

add_library(plInterp STATIC ${plInterp_SOURCES} ${plInterp_HEADERS})

target_link_libraries(plInterp pnFactory)
target_link_libraries(plInterp pnNetCommon)
target_link_libraries(plInterp plIntersect)
target_link_libraries(plInterp plTransform)

source_group("Source Files" FILES ${plInterp_SOURCES})
source_group("Header Files" FILES ${plInterp_HEADERS})
