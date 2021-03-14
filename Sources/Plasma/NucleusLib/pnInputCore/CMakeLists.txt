set(pnInputCore_HEADERS
    plControlDefinition.h
    plControlEventCodes.h
    plInputMap.h
    plKeyDef.h
    plKeyMap.h
)

set(pnInputCore_SOURCES
    plInputMap.cpp
)

plasma_library(pnInputCore SOURCES ${pnInputCore_HEADERS} ${pnInputCore_SOURCES})
target_link_libraries(
    pnInputCore
    PUBLIC
        CoreLib
    PRIVATE
        plResMgr # :(
)

target_include_directories(pnInputCore PRIVATE "${PLASMA_SOURCE_ROOT}/PubUtilLib")

source_group("Header Files" FILES ${pnInputCore_HEADERS})
source_group("Source Files" FILES ${pnInputCore_SOURCES})
