include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

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

if(HAVE_LIBSECRET)
    add_definitions(-DHAVE_LIBSECRET)
    set(pfPasswordStore_SOURCES ${pfPasswordStore_SOURCES}
        pfPasswordStore_Unix.cpp
    )
    include_directories(${LIBSECRET_INCLUDE_DIRS})
endif(HAVE_LIBSECRET)

if(APPLE)
    set(pfPasswordStore_SOURCES ${pfPasswordStore_SOURCES}
        pfPasswordStore_Apple.cpp
    )
endif(APPLE)

add_library(pfPasswordStore STATIC ${pfPasswordStore_HEADERS} ${pfPasswordStore_SOURCES})
target_link_libraries(pfPasswordStore CoreLib plFile pnNetBase)

if(APPLE)
    find_library(SECURITY_LIBRARY Security)
    target_link_libraries(pfPasswordStore ${SECURITY_LIBRARY})
endif(APPLE)

if(HAVE_LIBSECRET)
    target_link_libraries(pfPasswordStore ${LIBSECRET_LIBRARIES})
endif(HAVE_LIBSECRET)

source_group("Header Files" FILES ${pfPasswordStore_HEADERS})
source_group("Source Files" FILES ${pfPasswordStore_SOURCES})

