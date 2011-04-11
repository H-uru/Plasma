include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnIni_HEADERS
    Intern.h
    Pch.h
    pnIni.h
)

set(pnIni_PRIVATE
    Private/pnIniAllIncludes.h
    Private/pnIniChange.h
    Private/pnIniCore.h
    Private/pnIniCore.cpp
    Private/pnIniSrv.h
)

add_library(pnIni STATIC ${pnIni_HEADERS} ${pnIni_PRIVATE})

source_group("Header Files" FILES ${pnIni_HEADERS})
source_group("Private" FILES ${pnIni_PRIVATE})
