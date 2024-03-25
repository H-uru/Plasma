set(plClipboard_SOURCES
    plClipboard.cpp
)

set(plClipboard_HEADERS
    plClipboard.h
)

plasma_library(plClipboard SOURCES ${plClipboard_SOURCES} ${plClipboard_HEADERS})
target_link_libraries(
    plClipboard
    PUBLIC
        CoreLib
)

source_group("Source Files" FILES ${plClipboard_SOURCES})
source_group("Header Files" FILES ${plClipboard_HEADERS})
