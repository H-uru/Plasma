set(pfConsole_SOURCES
    pfAudioConsoleCommands.cpp
    pfAvatarConsoleCommands.cpp
    pfConsole.cpp
    pfConsoleCommandUtilities.cpp
    pfConsoleCommands.cpp
    pfConsoleCommandsNet.cpp
    pfConsoleDirSrc.cpp
    pfDispatchLog.cpp
    pfGameConsoleCommands.cpp
)

set(pfConsole_HEADERS
    pfConsole.h
    pfConsoleCommandUtilities.h
    pfConsoleCreatable.h
    pfConsoleDirSrc.h
    pfDispatchLog.h
)

plasma_library(pfConsole
    FOLDER FeatureLib
    SOURCES ${pfConsole_SOURCES} ${pfConsole_HEADERS}
    PRECOMPILED_HEADERS Pch.h
)
target_link_libraries(
    pfConsole
    PUBLIC
        CoreLib
        pnDispatch
        pnKeyedObject
        pfConsoleCore
    PRIVATE
        pnInputCore
        pnMessage
        pnNetCommon
        pnNucleusInc
        pnSceneObject
        pnUUID
        plAgeDescription
        plAgeLoader
        plAnimation
        plAudio
        plAvatar
        plClipboard
        plDrawable
        plGImage
        plGLight
        plInputCore
        plNetClient
        plNetCommon
        plNetGameLib
        plNetMessage
        plMessage
        plMessageBox
        plModifier
        plParticleSystem
        plPhysical
        plPhysX
        plPipeline
        pfPython
        plResMgr
        plScene
        plSDL
        plStatGather
        plStatusLog
        plSurface
        plUnifiedTime
        plVault
        pfAudio
        pfAnimation
        pfCamera
        pfJournalBook
        pfMessage
        pfPython
        pfSurface
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${pfConsole_SOURCES})
source_group("Header Files" FILES ${pfConsole_HEADERS})
