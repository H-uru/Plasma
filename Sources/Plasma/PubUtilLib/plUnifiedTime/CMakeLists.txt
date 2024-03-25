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

plasma_library(plUnifiedTime SOURCES ${plUnifiedTime_SOURCES} ${plUnifiedTime_HEADERS})
target_link_libraries(
    plUnifiedTime
    PUBLIC
        CoreLib
    PRIVATE
        pnNucleusInc
)

source_group("Source Files" FILES ${plUnifiedTime_SOURCES})
source_group("Header Files" FILES ${plUnifiedTime_HEADERS})
