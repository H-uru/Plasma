include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnNetBase_HEADERS
    pnNbConst.h
    pnNbError.h
    pnNbKeys.h
    pnNbProtocol.h
    pnNbSrvs.h
    pnNetBase.h
)

set(pnNetBase_SOURCES
    pnNbError.cpp
    pnNbKeys.cpp
    pnNbProtocol.cpp
    pnNbSrvs.cpp
)

add_library(pnNetBase STATIC ${pnNetBase_HEADERS} ${pnNetBase_SOURCES})

source_group("Header Files" FILES ${pnNetBase_HEADERS})
source_group("Source Files" FILES ${pnNetBase_SOURCES})
