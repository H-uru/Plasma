include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

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
