set(plClipboard_SOURCES
    plClipboard.cpp
    $<$<PLATFORM_ID:Darwin>:plClipboard_Mac.mm>
    $<$<PLATFORM_ID:Windows>:plClipboard_Win.cpp>
)

set(plClipboard_HEADERS
    plClipboard.h
)

plasma_library(plClipboard
    FOLDER PubUtilLib
    SOURCES ${plClipboard_SOURCES} ${plClipboard_HEADERS}
)
target_link_libraries(
    plClipboard
    PUBLIC
        CoreLib
    PRIVATE
        "$<$<PLATFORM_ID:Darwin>:-framework Cocoa>"
)

source_group("Source Files" FILES ${plClipboard_SOURCES})
source_group("Header Files" FILES ${plClipboard_HEADERS})
