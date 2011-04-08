include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)

set(pnFactory_SOURCES
    plFactory.cpp
)

set(pnFactory_HEADERS
    plCreatable.h
    plCreator.h
    plFactory.h
)

add_library(pnFactory STATIC ${pnFactory_SOURCES} ${pnFactory_HEADERS})

source_group("Source Files" FILES ${pnFactory_SOURCES})
source_group("Header Files" FILES ${pnFactory_HEADERS})