include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(pnInputCore_HEADERS
    plControlDefinition.h
    plControlEventCodes.h
    plInputMap.h
    plKeyDef.h
    plKeyMap.h
    plOSMsg.h
)

set(pnInputCore_SOURCES
    plInputMap.cpp
)

add_library(pnInputCore STATIC ${pnInputCore_HEADERS} ${pnInputCore_SOURCES})

source_group("Header Files" FILES ${pnInputCore_HEADERS})
source_group("Source Files" FILES ${pnInputCore_SOURCES})
