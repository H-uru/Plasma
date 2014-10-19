include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plUnifiedTime_SOURCES
    plClientUnifiedTime.cpp
    plTimeSpan.cpp
    plUnifiedTime.cpp
)

set(plUnifiedTime_HEADERS
    plClientUnifiedTime.h
    plTimeSpan.h
    plUnifiedTime.h
)

add_library(plUnifiedTime STATIC ${plUnifiedTime_SOURCES} ${plUnifiedTime_HEADERS})

target_link_libraries(plUnifiedTime CoreLib)

source_group("Source Files" FILES ${plUnifiedTime_SOURCES})
source_group("Header Files" FILES ${plUnifiedTime_HEADERS})
