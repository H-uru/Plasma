include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

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
