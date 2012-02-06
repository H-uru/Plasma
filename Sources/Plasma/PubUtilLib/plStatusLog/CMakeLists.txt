include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plStatusLog_SOURCES
    plEncryptLogLine.cpp
    plLoggable.cpp
    plStatusLog.cpp
)

set(plStatusLog_HEADERS
    plEncryptLogLine.h
    plLoggable.h
    plStatusLog.h
)

add_library(plStatusLog STATIC ${plStatusLog_SOURCES} ${plStatusLog_HEADERS})
target_link_libraries(plStatusLog CoreLib plFile plUnifiedTime pnProduct)

source_group("Source Files" FILES ${plStatusLog_SOURCES})
source_group("Header Files" FILES ${plStatusLog_HEADERS})
