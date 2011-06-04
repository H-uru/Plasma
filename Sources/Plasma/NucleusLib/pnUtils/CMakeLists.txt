include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnUtils_HEADERS
    Intern.h
    Pch.h
    pnUtils.h
)

set(pnUtils_PRIVATE
    Private/pnUtAddr.h
    Private/pnUtAddr.cpp
    Private/pnUtAllIncludes.h
    Private/pnUtArray.h
    Private/pnUtArray.cpp
    Private/pnUtBase64.h
    Private/pnUtBase64.cpp
    Private/pnUtBigNum.h
    Private/pnUtBigNum.cpp
    Private/pnUtCmd.h
    Private/pnUtCmd.cpp
    Private/pnUtCoreLib.h
    Private/pnUtCrypt.h
    Private/pnUtCrypt.cpp
    Private/pnUtEndian.h
    Private/pnUtEndian.cpp
    Private/pnUtHash.h
    Private/pnUtHash.cpp
    Private/pnUtList.h
    Private/pnUtList.cpp
    Private/pnUtMath.h
    Private/pnUtMath.cpp
    Private/pnUtMisc.h
    Private/pnUtMisc.cpp
    Private/pnUtPath.h
    Private/pnUtPath.cpp
    Private/pnUtPragma.h
    Private/pnUtPriQ.h
    Private/pnUtRand.h
    Private/pnUtRand.cpp
    Private/pnUtRef.h
    Private/pnUtSkipList.h
    Private/pnUtSort.h
    Private/pnUtSpareList.h
    Private/pnUtSpareList.cpp
    Private/pnUtStr.h
    Private/pnUtStr.cpp
    Private/pnUtSubst.h
    Private/pnUtSubst.cpp
    Private/pnUtSync.h
    Private/pnUtTime.h
    Private/pnUtTime.cpp
    Private/pnUtTls.h
    Private/pnUtTls.cpp
    Private/pnUtUuid.h
    Private/pnUtUuid.cpp
)

set(pnUtils_SOURCES
    pnUtils.cpp
)

if(WIN32)
    set(pnUtils_WIN32
        Private/Win32/pnUtW32Addr.cpp
        Private/Win32/pnUtW32Misc.cpp
        Private/Win32/pnUtW32Path.cpp
        Private/Win32/pnUtW32Str.cpp
        Private/Win32/pnUtW32Sync.cpp
        Private/Win32/pnUtW32Time.cpp
        Private/Win32/pnUtW32Uuid.cpp
    )
else()
    set(pnUtils_UNIX
        Private/Unix/pnUtUxStr.cpp
        Private/Unix/pnUtUxSync.cpp
        Private/Unix/pnUtUxUuid.cpp
    )
endif()

add_library(pnUtils STATIC ${pnUtils_HEADERS} ${pnUtils_PRIVATE} ${pnUtils_SOURCES}
                           ${pnUtils_UNIX} ${pnUtils_WIN32})

source_group("Header Files" FILES ${pnUtils_HEADERS})
source_group("Source Files" FILES ${pnUtils_SOURCES})
source_group("Private" FILES ${pnUtils_PRIVATE})
if(WIN32)
    source_group("Private\\Win32" FILES ${pnUtils_WIN32})
else()
    source_group("Private\\Unix" FILES ${pnUtils_UNIX})
endif()
