include_directories("../../CoreLib")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

include_directories(${CARES_INCLUDE_DIR})

set(plNetCore_SOURCES
    plNetCore.cpp
    plNetCoreDns.cpp
)

set(plNetCore_HEADERS
    plNetCore.h
    plNetCoreDns.h
)

add_library(plNetCore STATIC ${plNetCore_SOURCES} ${plNetCore_HEADERS})

target_link_libraries(plNetCore pnNetCommon)
target_link_libraries(plNetCore ${CARES_LIBRARIES})

if(WIN32)
    target_link_libraries(plNetCore Ws2_32)
endif(WIN32)

source_group("Source Files" FILES ${plNetCore_SOURCES})
source_group("Header Files" FILES ${plNetCore_HEADERS})
