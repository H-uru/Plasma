include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

set(plNetMessage_SOURCES
    plNetMessage.cpp
    plNetMsgHelpers.cpp
)

set(plNetMessage_HEADERS
    plNetCommonMessage.h
    plNetMessage.h
    plNetMessageCreatable.h
    plNetMsgHelpers.h
    plNetMsgVersion.h
)

add_library(plNetMessage STATIC ${plNetMessage_SOURCES} ${plNetMessage_HEADERS})

source_group("Source Files" FILES ${plNetMessage_SOURCES})
source_group("Header Files" FILES ${plNetMessage_HEADERS})
