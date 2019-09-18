include_directories(../../CoreLib)
include_directories(../../FeatureLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${PYTHON_INCLUDE_DIR})

set(pfConsole_SOURCES
    pfAvatarConsoleCommands.cpp
    pfConsole.cpp
    pfConsoleCommands.cpp
    pfConsoleCommandsNet.cpp
    pfConsoleDirSrc.cpp
    pfDispatchLog.cpp
    pfGameConsoleCommands.cpp
)

set(pfConsole_HEADERS
    pfConsole.h
    pfConsoleCreatable.h
    pfConsoleDirSrc.h
    pfDispatchLog.h
)

add_library(pfConsole STATIC ${pfConsole_SOURCES} ${pfConsole_HEADERS})

source_group("Source Files" FILES ${pfConsole_SOURCES})
source_group("Header Files" FILES ${pfConsole_HEADERS})
