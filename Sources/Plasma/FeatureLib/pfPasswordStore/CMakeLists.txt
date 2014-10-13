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

add_library(pfPasswordStore STATIC ${pfPasswordStore_HEADERS} ${pfPasswordStore_SOURCES})
target_link_libraries(pfPasswordStore CoreLib plFile)

if(APPLE)
    find_library(SECURITY_LIBRARY Security)
    target_link_libraries(pfPasswordStore ${SECURITY_LIBRARY})
endif(APPLE)

source_group("Header Files" FILES ${pfPasswordStore_HEADERS})
source_group("Source Files" FILES ${pfPasswordStore_SOURCES})

