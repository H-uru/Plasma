set(pnAsyncCoreExe_SOURCES
    pnAceCore.cpp
    pnAceDns.cpp
    pnAceSocket.cpp
    pnAceThread.cpp
    pnAceTimer.cpp
)

set(pnAysncCoreExe_PRIVATE
    Private/pnAceInt.h
)

plasma_library(pnAsyncCoreExe
    FOLDER NucleusLib
    SOURCES ${pnAsyncCoreExe_SOURCES} ${pnAsyncCoreExe_PRIVATE}
    PRECOMPILED_HEADERS Pch.h
)

# Yeah, this looks strange, but this library has no public headers. It's
# the actual implementation of pnAsyncCore. Either rewrite it or get over it.
target_link_libraries(
    pnAsyncCoreExe
    PRIVATE
        CoreLib
        pnAsyncCore
        pnNetCommon
        pnNucleusInc
        ASIO::ASIO
        $<$<BOOL:${USE_VLD}>:${VLD_LIBRARY}>
)

source_group("Source Files" FILES ${pnAsyncCoreExe_SOURCES})
source_group("Private" FILES ${pnAsyncCoreExe_PRIVATE})
