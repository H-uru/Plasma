include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plNetTransport_SOURCES
    plNetTransport.cpp
    plNetTransportMember.cpp
)

set(plNetTransport_HEADERS
    plNetTransport.h
    plNetTransportMember.h
)

add_library(plNetTransport STATIC ${plNetTransport_SOURCES} ${plNetTransport_HEADERS})

source_group("Source Files" FILES ${plNetTransport_SOURCES})
source_group("Header Files" FILES ${plNetTransport_HEADERS})
