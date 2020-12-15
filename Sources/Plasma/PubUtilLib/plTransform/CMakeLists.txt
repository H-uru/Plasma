include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

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
