set(plStatusLog_SOURCES
    plEncryptLogLine.cpp
    plStatusLog.cpp
)

set(plStatusLog_HEADERS
    plEncryptLogLine.h
    plStatusLog.h
)

plasma_library(plStatusLog
    FOLDER PubUtilLib
    SOURCES ${plStatusLog_SOURCES} ${plStatusLog_HEADERS}
)
target_link_libraries(
    plStatusLog
    PUBLIC
        CoreLib
        pnNucleusInc
    PRIVATE
        plUnifiedTime
)

source_group("Source Files" FILES ${plStatusLog_SOURCES})
source_group("Header Files" FILES ${plStatusLog_HEADERS})
