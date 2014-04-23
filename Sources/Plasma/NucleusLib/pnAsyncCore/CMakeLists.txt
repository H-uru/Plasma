include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../PubUtilLib)

set(pnAsyncCore_SOURCES
    pnAcCore.cpp
    pnAcIo.cpp
    pnAcInt.cpp
    pnAcLog.cpp
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

set(pnAysncCore_NT
    Nt/pnAcNt.cpp
    Nt/pnAcNtInt.h
    Nt/pnAcNtSocket.cpp
)

set(pnAsyncCore_WIN32
    Win32/pnAcW32Dns.cpp
)

if (WIN32)
    add_library(pnAsyncCore STATIC
            ${pnAsyncCore_PRIVATE} ${pnAsyncCore_HEADERS}
            ${pnAsyncCore_SOURCES} ${pnAysncCore_NT}
            ${pnAsyncCore_WIN32})
else ()
    message("NucleusLib/pnAsyncCore not implemented for this platform!")
endif ()

source_group("Source Files" FILES ${pnAsyncCore_SOURCES})
source_group("Header Files" FILES ${pnAsyncCore_HEADERS})
source_group("Private" FILES ${pnAsyncCore_PRIVATE})
source_group("Nt" FILES ${pnAysncCore_NT})
source_group("Win32" FILES ${pnAsyncCore_WIN32})
