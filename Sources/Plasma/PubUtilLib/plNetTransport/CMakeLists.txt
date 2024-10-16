set(plNetTransport_SOURCES
    plNetTransport.cpp
    plNetTransportMember.cpp
)

set(plNetTransport_HEADERS
    plNetTransport.h
    plNetTransportMember.h
)

plasma_library(plNetTransport
    FOLDER PubUtilLib
    SOURCES ${plNetTransport_SOURCES} ${plNetTransport_HEADERS}
)
target_link_libraries(
    plNetTransport
    PUBLIC
        CoreLib
        pnKeyedObject
        plNetCommon
    PRIVATE
        plNetClient
        plNetMessage
)

source_group("Source Files" FILES ${plNetTransport_SOURCES})
source_group("Header Files" FILES ${plNetTransport_HEADERS})
