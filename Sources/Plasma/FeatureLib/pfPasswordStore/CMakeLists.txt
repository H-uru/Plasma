set(pfPasswordStore_HEADERS
    pfPasswordStore.h
    pfPasswordStore_impl.h
)

set(pfPasswordStore_SOURCES
    pfPasswordStore.cpp
)

if(WIN32 AND NOT CYGWIN)
    list(APPEND pfPasswordStore_SOURCES pfPasswordStore_Win.cpp)
elseif(APPLE)
    list(APPEND pfPasswordStore_SOURCES pfPasswordStore_Apple.cpp)
elseif(TARGET PkgConfig::LIBSECRET)
    list(APPEND pfPasswordStore_SOURCES pfPasswordStore_Unix.cpp)
endif()

plasma_library(pfPasswordStore
    FOLDER FeatureLib
    SOURCES ${pfPasswordStore_HEADERS} ${pfPasswordStore_SOURCES}
)
target_link_libraries(
    pfPasswordStore
    PUBLIC
        CoreLib
    PRIVATE
        pnNetBase
        plFile
        "$<$<OR:$<PLATFORM_ID:Darwin>,$<PLATFORM_ID:iOS>>:-framework Security>"
)

if(WIN32 AND NOT CYGWIN)
    target_link_libraries(pfPasswordStore PRIVATE advapi32)
elseif(TARGET PkgConfig::LIBSECRET)
    target_compile_definitions(pfPasswordStore PRIVATE HAVE_LIBSECRET)
    target_link_libraries(pfPasswordStore PRIVATE PkgConfig::LIBSECRET)
endif()

source_group("Header Files" FILES ${pfPasswordStore_HEADERS})
source_group("Source Files" FILES ${pfPasswordStore_SOURCES})

