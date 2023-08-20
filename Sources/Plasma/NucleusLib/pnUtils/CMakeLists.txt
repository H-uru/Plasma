set(pnUtils_HEADERS
    pnUtStr.h
)

set(pnUtils_SOURCES
    pnUtStr.cpp
)

plasma_library(pnUtils SOURCES ${pnUtils_HEADERS} ${pnUtils_SOURCES})
target_link_libraries(
    pnUtils
    PUBLIC
        CoreLib
)

source_group("Header Files" FILES ${pnUtils_HEADERS})
source_group("Source Files" FILES ${pnUtils_SOURCES})
