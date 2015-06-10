include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnUtils_HEADERS
    Pch.h
    pnUtils.h
    pnUtCoreLib.h
    pnUtAllIncludes.h
    pnUtArray.h
    pnUtCrypt.h
    pnUtHash.h
    pnUtList.h
    pnUtPragma.h
    pnUtPriQ.h
    pnUtSort.h
    pnUtStr.h
    pnUtTime.h
)

set(pnUtils_SOURCES
    pnUtArray.cpp
    pnUtCrypt.cpp
    pnUtHash.cpp
    pnUtList.cpp
    pnUtStr.cpp
    pnUtTime.cpp
)

add_library(pnUtils STATIC ${pnUtils_HEADERS} ${pnUtils_SOURCES})

target_link_libraries(pnUtils CoreLib)

source_group("Header Files" FILES ${pnUtils_HEADERS})
source_group("Source Files" FILES ${pnUtils_SOURCES})
