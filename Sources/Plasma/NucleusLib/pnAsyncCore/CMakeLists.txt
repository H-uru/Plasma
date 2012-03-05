include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../PubUtilLib)

set(pnAsyncCore_HEADERS
    Pch.h
    pnAsyncCore.h
)

set(pnAsyncCore_PRIVATE
    Private/pnAcAllIncludes.h
    Private/pnAcCore.h
    Private/pnAcIo.h
    Private/pnAcLog.cpp
    Private/pnAcLog.h
    Private/pnAcThread.h
    Private/pnAcTimer.h
)

add_library(pnAsyncCore STATIC ${pnAsyncCore_PRIVATE} ${pnAsyncCore_HEADERS})

source_group("Header Files" FILES ${pnAsyncCore_HEADERS})
source_group("Private" FILES ${pnAsyncCore_PRIVATE})
