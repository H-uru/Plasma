include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

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

if(WIN32)
    include_directories(${DirectX_INCLUDE_DIR})

    set(plInputCore_HEADERS ${plInputCore_HEADERS}
        plDInputDevice.h
        plDInputManager.h
    )

    set(plInputCore_SOURCES ${plInputCore_SOURCES}
        plDInputDevice.cpp
        plDInputManager.cpp
    )
endif(WIN32)

add_library(plInputCore STATIC ${plInputCore_SOURCES} ${plInputCore_HEADERS})

target_link_libraries(plInputCore CoreLib)
target_link_libraries(plInputCore pnInputCore)

source_group("Source Files" FILES ${plInputCore_SOURCES})
source_group("Header Files" FILES ${plInputCore_HEADERS})
