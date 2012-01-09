include_directories("../../CoreLib")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

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

source_group("Source Files" FILES ${plAgeLoader_SOURCES})
source_group("Header Files" FILES ${plAgeLoader_HEADERS})
