include_directories(${PLASMA_SOURCE_ROOT}/CoreLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib/inc)

set(pnFactory_SOURCES
    plFactory.cpp
)

set(pnFactory_HEADERS
    plCreatable.h
    plCreator.h
    plFactory.h
)

add_library(pnFactory STATIC ${pnFactory_SOURCES} ${pnFactory_HEADERS})
target_link_libraries(pnFactory CoreLib)

source_group("Source Files" FILES ${pnFactory_SOURCES})
source_group("Header Files" FILES ${pnFactory_HEADERS})
