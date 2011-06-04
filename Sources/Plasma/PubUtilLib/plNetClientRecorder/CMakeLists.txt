include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plNetClientRecorder_SOURCES
    plNetClientRecorder.cpp
    plNetClientStatsRecorder.cpp
    plNetClientStreamRecorder.cpp
)

set(plNetClientRecorder_HEADERS
    plNetClientRecorder.h
)

add_library(plNetClientRecorder STATIC ${plNetClientRecorder_SOURCES} ${plNetClientRecorder_HEADERS})

source_group("Source Files" FILES ${plNetClientRecorder_SOURCES})
source_group("Header Files" FILES ${plNetClientRecorder_HEADERS})
