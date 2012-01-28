include_directories("../../Apps")
include_directories("../../CoreLib")
include_directories("../../FeatureLib/inc")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib/inc")
include_directories("../../PubUtilLib")

if(Bink_SDK_AVAILABLE)
    include_directories(${Bink_INCLUDE_DIR})
endif()
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${PYTHON_INCLUDE_DIR})
include_directories(${CURL_INCLUDE_DIR})

# Test for Python Interpreter, which will be used for extra build scripts if available
find_package(PythonInterp)
if(PYTHONINTERP_FOUND)
    include(FindPythonModule)
    # Test for Python modules needed to build resource.dat
    find_python_module(rsvg)
    find_python_module(Image)
    if((DEFINED PY_RSVG) AND (DEFINED PY_IMAGE))
        set(BUILD_RESOURCE_DAT ON)
    endif((DEFINED PY_RSVG) AND (DEFINED PY_IMAGE))
endif(PYTHONINTERP_FOUND)

add_subdirectory(external)

set(plClient_HEADERS
    plClient.h
    plClientCreatable.h
    #plClientKey.h
    plClientUpdateFormat.h
    #plPluginClient.h
)

set(plClient_SOURCES
    pfAllCreatables.cpp
    plAllCreatables.cpp
    plClient.cpp
    #plClientKey.cpp
    #plPluginClient.cpp
    pnAllCreatables.cpp
    winmain.cpp
)

set(plClient_TEXT
    ../../../../Docs/ReleaseNotes/ReleaseNotes.txt
)

set(plClient_RESOURCES
    res/plClient.rc
    res/resource.h
    res/banner.bmp
    res/cnsl1.bin
    res/Dirt.ICO
    res/headspin.ico
)

add_executable(plClient WIN32 ${plClient_SOURCES} ${plClient_HEADERS}
                              ${plClient_TEXT} ${plClient_RESOURCES})

if(BUILD_RESOURCE_DAT)
    add_dependencies(plClient externalResources)
endif(BUILD_RESOURCE_DAT)

if(PLASMA_EXTERNAL_RELEASE)
    set_target_properties(plClient PROPERTIES OUTPUT_NAME "UruExplorer")
endif(PLASMA_EXTERNAL_RELEASE)
target_link_libraries(plClient CoreLib)
target_link_libraries(plClient pfAnimation)
target_link_libraries(plClient pfAudio)
target_link_libraries(plClient pfCamera)
target_link_libraries(plClient pfCCR)
target_link_libraries(plClient pfCharacter)
target_link_libraries(plClient pfConditional)
target_link_libraries(plClient pfConsole)
target_link_libraries(plClient pfConsoleCore)
target_link_libraries(plClient pfCsrSrv)
target_link_libraries(plClient pfGameGUIMgr)
target_link_libraries(plClient pfGameMgr)
target_link_libraries(plClient pfGameScoreMgr)
target_link_libraries(plClient pfJournalBook)
target_link_libraries(plClient pfLocalizationMgr)
target_link_libraries(plClient pfMessage)
target_link_libraries(plClient pfPython)
target_link_libraries(plClient pfSecurePreloader)
target_link_libraries(plClient pfStackTrace)
target_link_libraries(plClient pfSurface)
target_link_libraries(plClient plAgeDescription)
target_link_libraries(plClient plAgeLoader)
target_link_libraries(plClient plAudible)
target_link_libraries(plClient plAudio)
target_link_libraries(plClient plAudioCore)
target_link_libraries(plClient plAvatar)
target_link_libraries(plClient plClientResMgr)
#target_link_libraries(plClient plClientKey)
target_link_libraries(plClient plCompression)
target_link_libraries(plClient plContainer)
target_link_libraries(plClient plDrawable)
target_link_libraries(plClient plEncryption)
target_link_libraries(plClient plFile)
target_link_libraries(plClient plGImage)
target_link_libraries(plClient plGLight)
target_link_libraries(plClient plInputCore)
target_link_libraries(plClient plInterp)
target_link_libraries(plClient plIntersect)
target_link_libraries(plClient plJPEG)
target_link_libraries(plClient plMath)
target_link_libraries(plClient plMessage)
target_link_libraries(plClient plModifier)
target_link_libraries(plClient plNetClient)
target_link_libraries(plClient plNetClientComm)
target_link_libraries(plClient plNetClientRecorder)
target_link_libraries(plClient plNetCommon)
target_link_libraries(plClient plNetGameLib)
target_link_libraries(plClient plNetMessage)
target_link_libraries(plClient plNetTransport)
target_link_libraries(plClient plParticleSystem)
target_link_libraries(plClient plPhysical)
target_link_libraries(plClient plPhysX)
target_link_libraries(plClient plPipeline)
target_link_libraries(plClient plProgressMgr)
target_link_libraries(plClient plResMgr)
target_link_libraries(plClient plScene)
target_link_libraries(plClient plSDL)
target_link_libraries(plClient plSockets)
target_link_libraries(plClient plStatGather)
target_link_libraries(plClient plStatusLog)
target_link_libraries(plClient plStreamLogger)
target_link_libraries(plClient plSurface)
target_link_libraries(plClient plTransform)
target_link_libraries(plClient plUnifiedTime)
target_link_libraries(plClient plVault)
target_link_libraries(plClient pnAddrInfo)
target_link_libraries(plClient pnAsyncCore)
target_link_libraries(plClient pnAsyncCoreExe)
target_link_libraries(plClient pnCsrNet)
target_link_libraries(plClient pnDispatch)
target_link_libraries(plClient pnFactory)
target_link_libraries(plClient pnGameMgr)
target_link_libraries(plClient pnIni)
target_link_libraries(plClient pnIniExe)
target_link_libraries(plClient pnInputCore)
target_link_libraries(plClient pnKeyedObject)
target_link_libraries(plClient pnMessage)
target_link_libraries(plClient pnModifier)
target_link_libraries(plClient pnNetBase)
target_link_libraries(plClient pnNetCli)
target_link_libraries(plClient pnNetCommon)
target_link_libraries(plClient pnNetProtocol)
target_link_libraries(plClient pnNucleusInc)
target_link_libraries(plClient pnProduct)
target_link_libraries(plClient pnSceneObject)
target_link_libraries(plClient pnSimpleNet)
target_link_libraries(plClient pnTimer)
target_link_libraries(plClient pnUtils)
target_link_libraries(plClient pnUtilsExe)
target_link_libraries(plClient pnUUID)

if(PYTHON_DEBUG_LIBRARY)
    target_link_libraries(plClient debug ${PYTHON_DEBUG_LIBRARY})
    target_link_libraries(plClient optimized ${PYTHON_LIBRARY})
else()
    target_link_libraries(plClient ${PYTHON_LIBRARY})
endif()

target_link_libraries(plClient ${OPENAL_LIBRARY})
target_link_libraries(plClient ${OPENSSL_LIBRARIES})
target_link_libraries(plClient ${EXPAT_LIBRARY})
target_link_libraries(plClient ${JPEG_LIBRARY})
target_link_libraries(plClient ${PNG_LIBRARY})
target_link_libraries(plClient ${Speex_LIBRARY})
target_link_libraries(plClient ${PHYSX_LIBRARIES})
target_link_libraries(plClient ${Ogg_LIBRARIES})
target_link_libraries(plClient ${Vorbis_LIBRARIES})
target_link_libraries(plClient ${DirectX_LIBRARIES})
target_link_libraries(plClient ${CURL_LIBRARY})

if(Bink_SDK_AVAILABLE)
    target_link_libraries(plClient ${Bink_LIBRARIES})
endif()

if (WIN32)
    target_link_libraries(plClient Rpcrt4)
    target_link_libraries(plClient Version)
    target_link_libraries(plClient Vfw32)
    target_link_libraries(plClient Ws2_32)
    target_link_libraries(plClient winmm)
    target_link_libraries(plClient strmiids)
endif(WIN32)

source_group("Source Files" FILES ${plClient_SOURCES})
source_group("Header Files" FILES ${plClient_HEADERS})
source_group("Text" FILES ${plClient_TEXT})
source_group("Resource Files" FILES ${plClient_RESOURCES})
