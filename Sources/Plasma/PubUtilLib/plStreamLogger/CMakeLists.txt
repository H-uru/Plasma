set(plStreamLogger_SOURCES
    plStreamLogger.cpp
)

set(plStreamLogger_HEADERS
    plStreamLogger.h
)

add_library(plStreamLogger STATIC ${plStreamLogger_SOURCES} ${plStreamLogger_HEADERS})
target_link_libraries(
    plStreamLogger
    PUBLIC
        CoreLib
        pnNetCommon
)

source_group("Source Files" FILES ${plStreamLogger_SOURCES})
source_group("Header Files" FILES ${plStreamLogger_HEADERS})
