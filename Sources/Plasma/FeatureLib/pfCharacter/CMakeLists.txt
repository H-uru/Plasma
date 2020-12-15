set(pfCharacter_SOURCES
    pfMarkerInfo.cpp
    pfMarkerMgr.cpp
)

set(pfCharacter_HEADERS
    pfCharacterCreatable.h
    pfMarkerInfo.h
    pfMarkerMgr.h
)

add_library(pfCharacter STATIC ${pfCharacter_SOURCES} ${pfCharacter_HEADERS})

source_group("Source Files" FILES ${pfCharacter_SOURCES})
source_group("Header Files" FILES ${pfCharacter_HEADERS})
