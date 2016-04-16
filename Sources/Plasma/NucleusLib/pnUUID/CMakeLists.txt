include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnUUID_SOURCES
    pnUUID.cpp
)

if(WIN32)
    set(pnUUID_SOURCES ${pnUUID_SOURCES} pnUUID_Win32.cpp)
endif(WIN32)

if(UNIX)
    if(NOT APPLE)
        find_package(Uuid REQUIRED)
        set_package_properties(Uuid PROPERTIES
            DESCRIPTION "UUID library for Linux (provided by util-linux)"
            TYPE REQUIRED
        )

        include_directories(${Uuid_INCLUDE_DIR})
    endif()

    set(pnUUID_SOURCES ${pnUUID_SOURCES} pnUUID_Unix.cpp)
endif(UNIX)

set(pnUUID_HEADERS
    pnUUID.h
)

add_library(pnUUID STATIC ${pnUUID_SOURCES} ${pnUUID_HEADERS})
target_link_libraries(pnUUID CoreLib)
if(WIN32)
    target_link_libraries(pnUUID pnUtils)
    target_link_libraries(pnUUID rpcrt4)
elseif(NOT APPLE)
    target_link_libraries(pnUUID ${Uuid_LIBRARIES})
endif(WIN32)

source_group("Source Files" FILES ${pnUUID_SOURCES})
source_group("Header Files" FILES ${pnUUID_HEADERS})
