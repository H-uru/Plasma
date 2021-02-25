set(pnAsyncCoreExe_SOURCES
    pnAceCore.cpp
    pnAceDns.cpp
    pnAceIo.cpp
    pnAceThread.cpp
    pnAceTimer.cpp
)

set(pnAsyncCoreExe_HEADERS
    Pch.h
)

set(pnAysncCoreExe_PRIVATE
    Private/pnAceInt.h
)

# I would do platform specific stuff here,
# but Pch.h just includes everything, so...

set(pnAsyncCoreExe_PRIVATE_NT
    Private/Nt/pnAceNt.cpp
    Private/Nt/pnAceNtInt.h
    Private/Nt/pnAceNtSocket.cpp
)

set(pnAsyncCoreExe_PRIVATE_WIN32
    Private/Win32/pnAceW32Thread.cpp
)

# End questionable part

add_library(pnAsyncCoreExe STATIC
            ${pnAsyncCoreExe_SOURCES} ${pnAsyncCoreExe_HEADERS}
            ${pnAsyncCoreExe_PRIVATE} ${pnAsyncCoreExe_PRIVATE_NT}
            ${pnAsyncCoreExe_PRIVATE_WIN32})

# Yeah, this looks strange, but this library has no public headers. It's
# the actual implementation of pnAsyncCore. Either rewrite it or get over it.
target_link_libraries(
    pnAsyncCoreExe
    PRIVATE
        CoreLib
        pnAsyncCore
        pnNetBase
        pnUtils
        ASIO::ASIO
        $<$<BOOL:${USE_VLD}>:${VLD_LIBRARY}>
)

source_group("Source Files" FILES ${pnAsyncCoreExe_SOURCES})
source_group("Header Files" FILES ${pnAsyncCoreExe_HEADERS})
source_group("Private" FILES ${pnAsyncCoreExe_PRIVATE})
source_group("Private\\Nt" FILES ${pnAysncCoreExe_PRIVATE_NT})
source_group("Private\\Win32" FILES ${pnAsyncCoreExe_PRIVATE_WIN32})
