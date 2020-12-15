include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

set(plStreamLogger_SOURCES
    plStreamLogger.cpp
)

set(plStreamLogger_HEADERS
    plStreamLogger.h
)

add_library(plStreamLogger STATIC ${plStreamLogger_SOURCES} ${plStreamLogger_HEADERS})

source_group("Source Files" FILES ${plStreamLogger_SOURCES})
source_group("Header Files" FILES ${plStreamLogger_HEADERS})
