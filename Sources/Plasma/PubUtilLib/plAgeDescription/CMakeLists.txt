include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

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
