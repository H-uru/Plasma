include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../NucleusLib/inc")
include_directories("../../PubUtilLib")

set(pfPasswordStore_HEADERS
    pfPasswordStore.h
    pfPasswordStore_impl.h
)

set(pfPasswordStore_SOURCES
    pfPasswordStore.cpp
)

if(WIN32 AND NOT CYGWIN)
    set(pfPasswordStore_SOURCES ${pfPasswordStore_SOURCES}
        pfPasswordStore_Win.cpp
    )
endif(WIN32 AND NOT CYGWIN)

if(UNIX)
    set(pfPasswordStore_SOURCES ${pfPasswordStore_SOURCES}
        pfPasswordStore_Unix.cpp
    )
endif(UNIX)

if(APPLE)
    set(pfPasswordStore_SOURCES ${pfPasswordStore_SOURCES}
        pfPasswordStore_Mac.cpp
    )
endif(APPLE)

add_library(pfPasswordStore STATIC ${pfPasswordStore_HEADERS} ${pfPasswordStore_SOURCES})
target_link_libraries(pfPasswordStore CoreLib plFile)

if(APPLE)
    find_library(SECURITY_LIBRARY Security)
    target_link_libraries(pfPasswordStore ${SECURITY_LIBRARY})
endif(APPLE)

source_group("Header Files" FILES ${pfPasswordStore_HEADERS})
source_group("Source Files" FILES ${pfPasswordStore_SOURCES})

