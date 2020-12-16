set(plStatusLog_SOURCES
    plEncryptLogLine.cpp
    plStatusLog.cpp
)

set(plStatusLog_HEADERS
    plEncryptLogLine.h
    plStatusLog.h
)

add_library(plStatusLog STATIC ${plStatusLog_SOURCES} ${plStatusLog_HEADERS})
target_link_libraries(plStatusLog CoreLib)
target_link_libraries(plStatusLog pnNucleusInc)
target_link_libraries(plStatusLog plFile)
target_link_libraries(plStatusLog plUnifiedTime)

source_group("Source Files" FILES ${plStatusLog_SOURCES})
source_group("Header Files" FILES ${plStatusLog_HEADERS})
