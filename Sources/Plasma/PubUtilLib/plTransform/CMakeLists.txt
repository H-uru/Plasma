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

plasma_library(plTransform SOURCES ${plTransform_SOURCES} ${plTransform_HEADERS})
target_link_libraries(
    plTransform
    PUBLIC
        CoreLib
    PRIVATE
        pnNucleusInc
        plInterp
)

source_group("Source Files" FILES ${plTransform_SOURCES})
source_group("Header Files" FILES ${plTransform_HEADERS})
