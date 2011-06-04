include_directories("../../CoreLib")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plAgeDescription_SOURCES
    plAgeDescription.cpp
    plAgeManifest.cpp
)

set(plAgeDescription_HEADERS
    plAgeDescription.h
    plAgeManifest.h
)

add_library(plAgeDescription STATIC ${plAgeDescription_SOURCES} ${plAgeDescription_HEADERS})

source_group("Source Files" FILES ${plAgeDescription_SOURCES})
source_group("Header Files" FILES ${plAgeDescription_HEADERS})
