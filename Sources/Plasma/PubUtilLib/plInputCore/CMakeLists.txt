include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

include_directories(${DirectX_INCLUDE_DIR})
include_directories(${OPENAL_INCLUDE_DIR})

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
