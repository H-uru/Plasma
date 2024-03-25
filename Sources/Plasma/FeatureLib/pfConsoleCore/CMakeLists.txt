set(pfConsoleCore_SOURCES
    pfConsoleCmd.cpp
    pfConsoleCommandsCore.cpp
    pfConsoleContext.cpp
    pfConsoleEngine.cpp
    pfConsoleParser.cpp
)

set(pfConsoleCore_HEADERS
    pfConsoleCmd.h
    pfConsoleContext.h
    pfConsoleEngine.h
    pfConsoleParser.h
)

plasma_library(pfConsoleCore SOURCES ${pfConsoleCore_SOURCES} ${pfConsoleCore_HEADERS})
target_link_libraries(
    pfConsoleCore
    PUBLIC
        CoreLib
    PRIVATE
        pnNetBase
        plFile
)

source_group("Source Files" FILES ${pfConsoleCore_SOURCES})
source_group("Header Files" FILES ${pfConsoleCore_HEADERS})
