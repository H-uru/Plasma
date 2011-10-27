include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnUUID_SOURCES
    pnUUID.cpp
)

if(WIN32)
    set(pnUUID_SOURCES ${pnUUID_SOURCES} pnUUID_Win32.cpp)
else(WIN32)
    set(pnUUID_SOURCES ${pnUUID_SOURCES} pnUUID_Unix.cpp)
endif(WIN32)

set(pnUUID_HEADERS
    pnUUID.h
    pnUUIDCreatable.h
)

add_library(pnUUID STATIC ${pnUUID_SOURCES} ${pnUUID_HEADERS})
target_link_libraries(pnUUID pnFactory)
if(WIN32)
    target_link_libraries(pnUUID pnUtils)
    target_link_libraries(pnUUID Rpcrt4)
else(WIN32)
    target_link_libraries(pnUUID uuid)
endif(WIN32)

source_group("Source Files" FILES ${pnUUID_SOURCES})
source_group("Header Files" FILES ${pnUUID_HEADERS})
