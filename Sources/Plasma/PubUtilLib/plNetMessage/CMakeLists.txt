set(plNetMessage_SOURCES
    plNetMessage.cpp
    plNetMsgHelpers.cpp
)

set(plNetMessage_HEADERS
    plNetMessage.h
    plNetMessageCreatable.h
    plNetMsgHelpers.h
    plNetMsgVersion.h
)

plasma_library(plNetMessage SOURCES ${plNetMessage_SOURCES} ${plNetMessage_HEADERS})
target_link_libraries(
    plNetMessage
    PUBLIC
        CoreLib
        pnFactory
        pnKeyedObject
        pnNetBase
        pnNetCommon
        plNetCommon
        plUnifiedTime
    PRIVATE
        pnMessage
        pnNucleusInc
        plCompression
        plSDL
)

source_group("Source Files" FILES ${plNetMessage_SOURCES})
source_group("Header Files" FILES ${plNetMessage_HEADERS})
