include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

include_directories(${OPENAL_INCLUDE_DIR})

if(WIN32)
    # NOTE: Includes of Windows SDK must be used BEFORE includes of DirectX SDK.
    #       Otherwise, it can make error about PVOID64!
    get_filename_component(WindowsSdkDir "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows;CurrentInstallFolder]" ABSOLUTE CACHE)
    include_directories("${WindowsSdkDir}/include" ${DirectX_INCLUDE_DIR})
    
endif(WIN32)

set(plInputCore_SOURCES
    plAvatarInputInterface.cpp
    plDebugInputInterface.cpp
    plDInputDevice.cpp
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
    plDInputDevice.h
    plInputCoreCreatable.h
    plInputDevice.h
    plInputInterface.h
    plInputInterfaceMgr.h
    plInputManager.h
    plSceneInputInterface.h
    plTelescopeInputInterface.h
)

add_library(plInputCore STATIC ${plInputCore_SOURCES} ${plInputCore_HEADERS})

source_group("Source Files" FILES ${plInputCore_SOURCES})
source_group("Header Files" FILES ${plInputCore_HEADERS})
