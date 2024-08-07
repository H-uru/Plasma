set(pfPasswordStore_HEADERS
    pfPasswordStore.h
    pfPasswordStore_impl.h
)

set(pfPasswordStore_SOURCES
    pfPasswordStore.cpp
)

if(WIN32 AND NOT CYGWIN)
    list(APPEND pfPasswordStore_SOURCES pfPasswordStore_Win.cpp)
elseif(TARGET PkgConfig::LIBSECRET AND NOT TARGET Security::Security)
    list(APPEND pfPasswordStore_SOURCES pfPasswordStore_Unix.cpp)
elseif(TARGET Security::Security)
    list(APPEND pfPasswordStore_SOURCES pfPasswordStore_Apple.cpp)
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
)

if(WIN32 AND NOT CYGWIN)
    target_link_libraries(pfPasswordStore PRIVATE advapi32)
elseif(TARGET PkgConfig::LIBSECRET AND NOT TARGET Security::Security)
    target_compile_definitions(pfPasswordStore PRIVATE HAVE_LIBSECRET)
    target_link_libraries(pfPasswordStore PRIVATE PkgConfig::LIBSECRET)
elseif(TARGET Security::Security)
    target_compile_definitions(pfPasswordStore PRIVATE HAVE_SECURITY)
    target_link_libraries(pfPasswordStore PRIVATE Security::Security)
endif()

source_group("Header Files" FILES ${pfPasswordStore_HEADERS})
source_group("Source Files" FILES ${pfPasswordStore_SOURCES})

