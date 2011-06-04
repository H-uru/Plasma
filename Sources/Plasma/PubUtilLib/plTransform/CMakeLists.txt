include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plTransform_SOURCES
    hsAffineParts.cpp
    hsEuler.cpp
    mat_decomp.cpp
)

set(plTransform_HEADERS
    hsAffineParts.h
    hsEuler.h
    mat_decomp.h
)

add_library(plTransform STATIC ${plTransform_SOURCES} ${plTransform_HEADERS})

source_group("Source Files" FILES ${plTransform_SOURCES})
source_group("Header Files" FILES ${plTransform_HEADERS})
