include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnUtils_HEADERS
    Pch.h
    pnUtils.h
    pnUtCoreLib.h
    pnUtAllIncludes.h
    pnUtArray.h
    pnUtCmd.h
    pnUtCrypt.h
    pnUtHash.h
    pnUtList.h
    pnUtMisc.h
    pnUtPath.h
    pnUtPragma.h
    pnUtPriQ.h
    pnUtRef.h
    pnUtSort.h
    pnUtStr.h
    pnUtTime.h
    pnUtUuid.h
)

set(pnUtils_SOURCES
    pnUtArray.cpp
    pnUtCmd.cpp
    pnUtCrypt.cpp
    pnUtHash.cpp
    pnUtList.cpp
    pnUtMisc.cpp
    pnUtPath.cpp
    pnUtStr.cpp
    pnUtTime.cpp
    pnUtUuid.cpp
)

if(WIN32)
    set(pnUtils_WIN32
        Win32/pnUtW32Misc.cpp
        Win32/pnUtW32Path.cpp
        Win32/pnUtW32Str.cpp
        Win32/pnUtW32Uuid.cpp
    )
else()
    set(pnUtils_UNIX
        Unix/pnUtUxStr.cpp
        Unix/pnUtUxUuid.cpp
    )
endif()

add_library(pnUtils STATIC ${pnUtils_HEADERS} ${pnUtils_SOURCES}
                           ${pnUtils_UNIX} ${pnUtils_WIN32})

source_group("Header Files" FILES ${pnUtils_HEADERS})
source_group("Source Files" FILES ${pnUtils_SOURCES})
if(WIN32)
    source_group("Win32" FILES ${pnUtils_WIN32})
else()
    source_group("Unix" FILES ${pnUtils_UNIX})
endif()
