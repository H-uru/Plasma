set(plNetClientComm_SOURCES
    plNetClientComm.cpp
)

set(plNetClientComm_HEADERS
    plNetClientComm.h
    plNetClientCommCreatable.h
)

add_library(plNetClientComm STATIC ${plNetClientComm_SOURCES} ${plNetClientComm_HEADERS})

target_include_directories(plNetClientComm PRIVATE "${PLASMA_SOURCE_ROOT}/FeatureLib")

source_group("Source Files" FILES ${plNetClientComm_SOURCES})
source_group("Header Files" FILES ${plNetClientComm_HEADERS})
