include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

set(plNetClientComm_SOURCES
    plNetClientComm.cpp
)

set(plNetClientComm_HEADERS
    plNetClientComm.h
    plNetClientCommCreatable.h
)

add_library(plNetClientComm STATIC ${plNetClientComm_SOURCES} ${plNetClientComm_HEADERS})

source_group("Source Files" FILES ${plNetClientComm_SOURCES})
source_group("Header Files" FILES ${plNetClientComm_HEADERS})
