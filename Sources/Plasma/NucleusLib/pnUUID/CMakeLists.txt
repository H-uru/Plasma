set(pnUUID_SOURCES
    pnUUID.cpp
    $<$<PLATFORM_ID:Windows>:pnUUID_Win32.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:pnUUID_Unix.cpp>
)

set(pnUUID_HEADERS
    pnUUID.h
)

if(UNIX AND NOT APPLE)
    set_package_properties(
        Uuid PROPERTIES
        DESCRIPTION "UUID library for Linux (provided by util-linux)"
        TYPE REQUIRED
    )
    find_package(Uuid REQUIRED)
endif()

add_library(pnUUID STATIC ${pnUUID_SOURCES} ${pnUUID_HEADERS})
target_link_libraries(
    pnUUID
    PUBLIC
        CoreLib
    PRIVATE
        $<$<PLATFORM_ID:Linux>:Uuid::Uuid>
        $<$<PLATFORM_ID:Windows>:rpcrt4>
)

source_group("Source Files" FILES ${pnUUID_SOURCES})
source_group("Header Files" FILES ${pnUUID_HEADERS})
