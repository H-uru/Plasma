include_directories(${PLASMA_SOURCE_ROOT}/CoreLib)
include_directories(${PLASMA_SOURCE_ROOT}/FeatureLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib)
include_directories(${PLASMA_SOURCE_ROOT}/PubUtilLib)

set(pfConsoleCore_SOURCES
    pfConsoleCmd.cpp
    pfConsoleCommandsCore.cpp
    pfConsoleContext.cpp
    pfConsoleEngine.cpp
)

set(pfConsoleCore_HEADERS
    pfConsoleCmd.h
    pfConsoleContext.h
    pfConsoleEngine.h
)

add_library(pfConsoleCore STATIC ${pfConsoleCore_SOURCES} ${pfConsoleCore_HEADERS})

source_group("Source Files" FILES ${pfConsoleCore_SOURCES})
source_group("Header Files" FILES ${pfConsoleCore_HEADERS})
