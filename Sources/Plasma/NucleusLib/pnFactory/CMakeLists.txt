set(pnFactory_SOURCES
    plFactory.cpp
)

set(pnFactory_HEADERS
    plCreatable.h
    plCreator.h
    plFactory.h
)

plasma_library(pnFactory
    FOLDER NucleusLib
    SOURCES ${pnFactory_SOURCES} ${pnFactory_HEADERS}
)
target_link_libraries(
    pnFactory
    PUBLIC
        CoreLib
        pnNucleusInc
)

source_group("Source Files" FILES ${pnFactory_SOURCES})
source_group("Header Files" FILES ${pnFactory_HEADERS})
