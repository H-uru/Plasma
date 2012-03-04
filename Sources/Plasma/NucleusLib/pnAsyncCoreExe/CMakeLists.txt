include_directories(../../CoreLib)
include_directories(../../NucleusLib)

set(pnAsyncCoreExe_SOURCES
    pnAceCore.cpp
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

set(pnAysncCoreExe_PRIVATE_NT
    Private/Nt/pnAceNt.cpp
    Private/Nt/pnAceNt.h
    Private/Nt/pnAceNtInt.h
    Private/Nt/pnAceNtSocket.cpp
    Private/Nt/pnAceNtThread.cpp
)

set(pnAsyncCoreExe_PRIVATE_UNIX
    Private/Unix/pnAceUx.h
)

set(pnAsyncCoreExe_PRIVATE_W9X
    Private/W9x/pnAceW9x.cpp
    Private/W9x/pnAceW9x.h
    Private/W9x/pnAceW9xInt.h
    Private/W9x/pnAceW9xSocket.cpp
    Private/W9x/pnAceW9xThread.cpp
)

set(pnAsyncCoreExe_PRIVATE_WIN32
    Private/Win32/pnAceW32Dns.cpp
    Private/Win32/pnAceW32Thread.cpp
)

# End questionable part

add_library(pnAsyncCoreExe STATIC
            ${pnAsyncCoreExe_SOURCES} ${pnAsyncCoreExe_HEADERS}
            ${pnAsyncCoreExe_PRIVATE} ${pnAysncCoreExe_PRIVATE_NT}
            ${pnAsyncCoreExe_PRIVATE_UNIX} ${pnAsyncCoreExe_PRIVATE_W9X}
            ${pnAsyncCoreExe_PRIVATE_WIN32})

source_group("Source Files" FILES ${pnAsyncCoreExe_SOURCES})
source_group("Header Files" FILES ${pnAsyncCoreExe_HEADERS})
source_group("Private" FILES ${pnAsyncCoreExe_PRIVATE})
source_group("Private\\Nt" FILES ${pnAysncCoreExe_PRIVATE_NT})
source_group("Private\\Unix" FILES ${pnAsyncCoreExe_PRIVATE_UNIX})
source_group("Private\\W9x" FILES ${pnAsyncCoreExe_PRIVATE_W9X})
source_group("Private\\Win32" FILES ${pnAsyncCoreExe_PRIVATE_WIN32})
