include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plStreamLogger_SOURCES
    plStreamLogger.cpp
)

set(plStreamLogger_HEADERS
    plStreamLogger.h
)

add_library(plStreamLogger STATIC ${plStreamLogger_SOURCES} ${plStreamLogger_HEADERS})

source_group("Source Files" FILES ${plStreamLogger_SOURCES})
source_group("Header Files" FILES ${plStreamLogger_HEADERS})
