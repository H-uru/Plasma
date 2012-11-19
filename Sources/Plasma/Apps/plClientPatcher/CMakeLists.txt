add_definitions(-D_LIB)

include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

include_directories(${OPENSSL_INCLUDE_DIR})

set(plClientPatcher_HEADERS
    Intern.h
    Pch.h
    UruPlayer.h
)

set(plClientPatcher_SOURCES
    UruPlayer.cpp
)

add_library(plClientPatcher STATIC ${plClientPatcher_HEADERS} ${plClientPatcher_SOURCES})
target_link_libraries(plClientPatcher CoreLib plAudioCore plStatusLog)

if(USE_VLD)
    target_link_libraries(plClientPatcher ${VLD_LIBRARY})
endif()

source_group("Header Files" FILES ${plClientPatcher_HEADERS})
source_group("Source Files" FILES ${plClientPatcher_SOURCES})
