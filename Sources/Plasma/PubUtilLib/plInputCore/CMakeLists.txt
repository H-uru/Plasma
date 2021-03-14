set(plInputCore_SOURCES
    plAvatarInputInterface.cpp
    plDebugInputInterface.cpp
    plInputDevice.cpp
    plInputInterface.cpp
    plInputInterfaceMgr.cpp
    plInputManager.cpp
    plSceneInputInterface.cpp
    plTelescopeInputInterface.cpp
)

set(plInputCore_HEADERS
    plAvatarInputInterface.h
    plDebugInputInterface.h
    plInputCoreCreatable.h
    plInputDevice.h
    plInputInterface.h
    plInputInterfaceMgr.h
    plInputManager.h
    plSceneInputInterface.h
    plTelescopeInputInterface.h
)

plasma_library(plInputCore SOURCES ${plInputCore_SOURCES} ${plInputCore_HEADERS})
target_link_libraries(
    plInputCore
    PUBLIC
        CoreLib
        pnInputCore
        pnKeyedObject
        pnModifier
        pnUUID
    PRIVATE
        pnMessage
        pnNetCommon
        pnNucleusInc
        pnSceneObject
        plAudio
        plAvatar
        plGImage
        plMessage
        plModifier
        plNetClient
        plNetTransport
        plPipeline
        plResMgr
        plVault
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${plInputCore_SOURCES})
source_group("Header Files" FILES ${plInputCore_HEADERS})
