set(pnAsyncCoreExe_SOURCES
    pnAceCore.cpp
    pnAceDns.cpp
    pnAceSocket.cpp
    pnAceThread.cpp
    pnAceTimer.cpp
)

set(pnAsyncCoreExe_HEADERS
    Pch.h
)

set(pnAysncCoreExe_PRIVATE
    Private/pnAceInt.h
)

plasma_library(pnAsyncCoreExe
    SOURCES ${pnAsyncCoreExe_SOURCES} ${pnAsyncCoreExe_HEADERS} ${pnAsyncCoreExe_PRIVATE}
)

# Yeah, this looks strange, but this library has no public headers. It's
# the actual implementation of pnAsyncCore. Either rewrite it or get over it.
target_link_libraries(
    pnAsyncCoreExe
    PRIVATE
        CoreLib
        pnAsyncCore
        pnNetBase
        pnNucleusInc
        ASIO::ASIO
        $<$<BOOL:${USE_VLD}>:${VLD_LIBRARY}>
)

source_group("Source Files" FILES ${pnAsyncCoreExe_SOURCES})
source_group("Header Files" FILES ${pnAsyncCoreExe_HEADERS})
source_group("Private" FILES ${pnAsyncCoreExe_PRIVATE})
