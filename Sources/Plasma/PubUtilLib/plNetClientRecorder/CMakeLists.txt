set(plNetClientRecorder_SOURCES
    plNetClientRecorder.cpp
    plNetClientStatsRecorder.cpp
    plNetClientStreamRecorder.cpp
)

set(plNetClientRecorder_HEADERS
    plNetClientRecorder.h
)

plasma_library(plNetClientRecorder SOURCES ${plNetClientRecorder_SOURCES} ${plNetClientRecorder_HEADERS})
target_link_libraries(
    plNetClientRecorder
    PUBLIC
        CoreLib
    PRIVATE
        pnMessage
        pnNetCommon
        pnNucleusInc
        plMessage
        plNetMessage
        plSDL
        plStatusLog
)

source_group("Source Files" FILES ${plNetClientRecorder_SOURCES})
source_group("Header Files" FILES ${plNetClientRecorder_HEADERS})
