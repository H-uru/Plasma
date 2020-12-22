set(pfConsole_SOURCES
    pfAudioConsoleCommands.cpp
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
target_link_libraries(
    pfConsole
    PUBLIC
        CoreLib
        pfConsoleCore
        pnDispatch
        pnFactory
        pnKeyedObject

    PRIVATE
        plAgeDescription
        plAgeLoader
        pfAnimation
        plAnimation
        pfAudio
        plAudio
        plAvatar
        plDrawable
        pfCamera
        pfGameGUIMgr
        plGImage
        plGLight
        plInputCore
        pnInputCore
        plNetClient
        plNetCommon
        pnNetCommon
        plNetGameLib
        pfMessage
        plMessage
        pnMessage
        plModifier
        plParticleSystem
        plPhysical
        plPhysX
        plPipeline
        pfPython
        plResMgr
        plScene
        pnSceneObject
        plSDL
        plStatGather
        plStatusLog
        plSurface
        pfSurface
        pnTimer
        plUnifiedTime
        plVault
        string_theory
)

source_group("Source Files" FILES ${pfConsole_SOURCES})
source_group("Header Files" FILES ${pfConsole_HEADERS})
