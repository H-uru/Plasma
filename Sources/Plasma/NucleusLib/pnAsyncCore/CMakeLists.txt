set(pnAsyncCore_HEADERS
    pnAcCore.h
    pnAcIo.h
    pnAcLog.h
    pnAcThread.h
    pnAcTimer.h
    pnAsyncCore.h
    pnAsyncCore_Private.h
)

set(pnAsyncCore_SOURCES
    pnAcCore.cpp
    pnAcDns.cpp
    pnAcLog.cpp
    pnAcSocket.cpp
    pnAcThread.cpp
    pnAcTimer.cpp
)

plasma_library(pnAsyncCore
    FOLDER NucleusLib
    SOURCES ${pnAsyncCore_HEADERS} ${pnAsyncCore_SOURCES}
    PRECOMPILED_HEADERS Pch.h
)

target_include_directories(pnAsyncCore PRIVATE "${PLASMA_SOURCE_ROOT}/PubUtilLib")
target_link_libraries(
    pnAsyncCore
    PUBLIC
        CoreLib
    PRIVATE
        pnNetCommon
        pnNucleusInc
        plStatusLog # :(
        ASIO::ASIO
)

source_group("Header Files" FILES ${pnAsyncCore_HEADERS})
source_group("Source Files" FILES ${pnAsyncCore_SOURCES})
