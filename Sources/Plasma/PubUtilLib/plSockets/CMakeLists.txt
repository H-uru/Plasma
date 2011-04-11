include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plSockets_SOURCES
    plBufferedSocketReader.cpp
    plBufferedSocketWriter.cpp
    plFdSet.cpp
    plMemBuffer.cpp
    plNet.cpp
    plOutgoingUdpSocket.cpp
    plRingBuffer.cpp
    plSocket.cpp
    plTcpListenSocket.cpp
    plTcpSocket.cpp
)

set(plSockets_HEADERS
    plBufferedSocketReader.h
    plBufferedSocketWriter.h
    plFdSet.h
    plMemBuffer.h
    plNet.h
    plOutgoingUdpSocket.h
    plRingBuffer.h
    plSocket.h
    plTcpListenSocket.h
    plTcpSocket.h
)

add_library(plSockets STATIC ${plSockets_SOURCES} ${plSockets_HEADERS})

source_group("Source Files" FILES ${plSockets_SOURCES})
source_group("Header Files" FILES ${plSockets_HEADERS})
