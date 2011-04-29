include_directories("../../CoreLib")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

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
