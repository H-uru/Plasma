include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plUUID_SOURCES
    plUUID.cpp
)

if(WIN32)
    set(plUUID_SOURCES ${plUUID_SOURCES} plUUID_Win32.cpp)
else(WIN32)
    set(plUUID_SOURCES ${plUUID_SOURCES} plUUID_Unix.cpp)
endif(WIN32)

set(plUUID_HEADERS
    plUUID.h
)

add_library(plUUID STATIC ${plUUID_SOURCES} ${plUUID_HEADERS})

source_group("Source Files" FILES ${plUUID_SOURCES})
source_group("Header Files" FILES ${plUUID_HEADERS})
