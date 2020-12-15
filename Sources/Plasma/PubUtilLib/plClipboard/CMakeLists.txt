include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

set(plClipboard_SOURCES
    plClipboard.cpp
)

set(plClipboard_HEADERS
    plClipboard.h
)

add_library(plClipboard STATIC ${plClipboard_SOURCES} ${plClipboard_HEADERS})

source_group("Source Files" FILES ${plClipboard_SOURCES})
source_group("Header Files" FILES ${plClipboard_HEADERS})
