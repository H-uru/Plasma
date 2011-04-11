include_directories(../../CoreLib)
include_directories(../../FeatureLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

set(pfCsrSrv_SOURCES
    pfCsrSrv.cpp
)

set(pfCsrSrv_HEADERS
    Pch.h
    pfCsrSrv.h
)

add_library(pfCsrSrv STATIC ${pfCsrSrv_SOURCES} ${pfCsrSrv_HEADERS})

source_group("Source Files" FILES ${pfCsrSrv_SOURCES})
source_group("Header Files" FILES ${pfCsrSrv_HEADERS})
