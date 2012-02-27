include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plClipboard_SOURCES
    plClipboard.cpp
)

set(plClipboard_HEADERS
    plClipboard.h
)

add_library(plClipboard STATIC ${plClipboard_SOURCES} ${plClipboard_HEADERS})

source_group("Source Files" FILES ${plClipboard_SOURCES})
source_group("Header Files" FILES ${plClipboard_HEADERS})
