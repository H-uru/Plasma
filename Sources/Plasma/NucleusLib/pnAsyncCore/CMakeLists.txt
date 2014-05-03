include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../PubUtilLib)

set(pnAsyncCore_SOURCES
    pnAcCore.cpp
    pnAcDns.cpp
    pnAcIo.cpp
    pnAcInt.cpp
    pnAcLog.cpp
    pnAcSocket.cpp
    pnAcTimer.cpp
)

set(pnAsyncCore_HEADERS
    pnAcCore.h
    pnAcSocket.h
    pnAcLog.h
    pnAcSocket.h
    pnAcTimer.h
)

set(pnAsyncCore_PRIVATE
    pnAcInt.h
)

add_library(pnAsyncCore STATIC
        ${pnAsyncCore_PRIVATE} ${pnAsyncCore_HEADERS}
        ${pnAsyncCore_SOURCES}
)

source_group("Source Files" FILES ${pnAsyncCore_SOURCES})
source_group("Header Files" FILES ${pnAsyncCore_HEADERS})
source_group("Private" FILES ${pnAsyncCore_PRIVATE})
