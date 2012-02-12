include_directories("../../Apps")
include_directories("../../CoreLib")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${CURL_INCLUDE_DIR})

set(plUruLauncher_HEADERS
    Intern.h
    Pch.h
    plLauncherInfo.h
)

set(plUruLauncher_SOURCES
    Main.cpp
    SelfPatcher.cpp
)

set(plUruLauncher_RESOURCES
    plUruLauncher.rc
    resource.h

    banner.bmp
    Dirt.ICO
)

add_executable(plUruLauncher WIN32 ${plUruLauncher_SOURCES} ${plUruLauncher_HEADERS}
                                   ${plUruLauncher_RESOURCES})
if(PLASMA_EXTERNAL_RELEASE)
    set_target_properties(plUruLauncher PROPERTIES OUTPUT_NAME UruLauncher)
endif(PLASMA_EXTERNAL_RELEASE)
target_link_libraries(plUruLauncher CoreLib)
target_link_libraries(plUruLauncher pfConsoleCore)
target_link_libraries(plUruLauncher plAudioCore)
target_link_libraries(plUruLauncher plClientPatcher)
target_link_libraries(plUruLauncher plCompression)
target_link_libraries(plUruLauncher plFile)
target_link_libraries(plUruLauncher plNetClient)
target_link_libraries(plUruLauncher plNetClientComm)
target_link_libraries(plUruLauncher plNetGameLib)
target_link_libraries(plUruLauncher plNetMessage)
target_link_libraries(plUruLauncher plNetTransport)
target_link_libraries(plUruLauncher plUnifiedTime)
target_link_libraries(plUruLauncher pnAsyncCore)
target_link_libraries(plUruLauncher pnAsyncCoreExe)
target_link_libraries(plUruLauncher pnEncryption)
target_link_libraries(plUruLauncher pnNetBase)
target_link_libraries(plUruLauncher pnNetCli)
target_link_libraries(plUruLauncher pnNetCommon)
target_link_libraries(plUruLauncher pnNetProtocol)
target_link_libraries(plUruLauncher pnProduct)
target_link_libraries(plUruLauncher pnSimpleNet)
target_link_libraries(plUruLauncher pnTimer)
target_link_libraries(plUruLauncher pnUtils)
target_link_libraries(plUruLauncher pnUtilsExe)

target_link_libraries(plUruLauncher ${DirectX_LIBRARIES})
target_link_libraries(plUruLauncher ${OPENSSL_LIBRARIES})
target_link_libraries(plUruLauncher ${Ogg_LIBRARIES})
target_link_libraries(plUruLauncher ${Vorbis_LIBRARIES})
target_link_libraries(plUruLauncher ${CURL_LIBRARY})
target_link_libraries(plUruLauncher ws2_32)
target_link_libraries(plUruLauncher rpcrt4)
target_link_libraries(plUruLauncher comctl32)

source_group("Source Files" FILES ${plUruLauncher_SOURCES})
source_group("Header Files" FILES ${plUruLauncher_HEADERS})
source_group("Resource Files" FILES ${plUruLauncher_RESOURCES})
