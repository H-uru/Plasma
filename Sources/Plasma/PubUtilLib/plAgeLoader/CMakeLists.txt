include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

set(plAgeLoader_SOURCES
    plAgeLoader.cpp
    plAgeLoaderPaging.cpp
    plResPatcher.cpp
)

set(plAgeLoader_HEADERS
    plAgeLoader.h
    plAgeLoaderCreatable.h
    plResPatcher.h
)

add_library(plAgeLoader STATIC ${plAgeLoader_SOURCES} ${plAgeLoader_HEADERS})

target_link_libraries(plAgeLoader pfPatcher)

source_group("Source Files" FILES ${plAgeLoader_SOURCES})
source_group("Header Files" FILES ${plAgeLoader_HEADERS})
