include_directories("../")
include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/FeatureLib")
include_directories("../../Plasma/FeatureLib/inc")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/PubUtilLib")
include_directories("../../Plasma/PubUtilLib/inc")
include_directories(${3dsm_INCLUDE_DIR})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${PHYSX_INCLUDE_DIRS})
include_directories(${PYTHON_INCLUDE_DIR})

set(MaxMain_HEADERS
    GlobalUtility.h
    main.def
    MaxAllocDll.h
    MaxCompat.h
    plActionTableMgr.h
    plAgeDescInterface.h
    plCommonObjLib.h
    plComponentDlg.h
    plComponentPanel.h
    plGetLocationDlg.h
    plMaxAccelerators.h
    plMaxCFGFile.h
    plMaxMenu.h
    plMaxMeshExtractor.h
    plMaxNode.h
    plMaxNodeBase.h
    plMaxNodeData.h
    plMaxUtils.h
    plMtlCollector.h
    plNodeLock.h
    plPhysicalProps.h
    plPhysXCooking.h
    plPlasmaRefMsgs.h
    plPluginResManager.h
    plPythonMgr.h
    plResCollector.h
    plResetXform.h
    plSaveSelected.h
    plTextureExportLog.h
    plTextureSearch.h
)

set(MaxMain_RESOURCES
    resource.h
    SimpleExport.rc
)

set(MaxMain_SOURCES
    GlobalUtility.cpp
    indexes.cpp
    main.cpp
    MaxAllocDll.cpp
    plActionTableMgr.cpp
    plAgeDescInterface.cpp
    plCommonObjLib.cpp
    plComponentDlg.cpp
    plComponentPanel.cpp
    plGetLocationDlg.cpp
    plMaxAccelerators.cpp
    plMaxCFGFile.cpp
    plMaxMenu.cpp
    plMaxMeshExtractor.cpp
    plMaxNode.cpp
    plMaxNodeBase.cpp
    plMaxUtils.cpp
    plMtlCollector.cpp
    plNodeLock.cpp
    plPhysicalProps.cpp
    plPhysXCooking.cpp
    plPluginResManager.cpp
    plPythonMgr.cpp
    plResCollector.cpp
    plResetXform.cpp
    plSaveSelected.cpp
    plTextureExportLog.cpp
    plTextureSearch.cpp
)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Plasma_BINARY_DIR}/bin)
add_library(MaxMain SHARED ${MaxMain_HEADERS} ${MaxMain_RESOURCES} ${MaxMain_SOURCES})
set_target_properties(MaxMain PROPERTIES OUTPUT_NAME "PlasmaMax")
set_target_properties(MaxMain PROPERTIES SUFFIX ".gup")

target_link_libraries(MaxMain MaxComponent)
target_link_libraries(MaxMain MaxConvert)
target_link_libraries(MaxMain MaxExport)
target_link_libraries(MaxMain MaxPlasmaMtls)
target_link_libraries(MaxMain ${3dsm_LIBRARIES})

if(Bink_SDK_AVAILABLE)
    target_link_libraries(MaxMain ${Bink_LIBRARIES})
endif()
target_link_libraries(MaxMain ${EXPAT_LIBRARY})
target_link_libraries(MaxMain ${DirectX_LIBRARIES})
target_link_libraries(MaxMain ${JPEG_LIBRARY})
target_link_libraries(MaxMain ${PNG_LIBRARY})
target_link_libraries(MaxMain ${Ogg_LIBRARIES})
target_link_libraries(MaxMain ${OPENAL_LIBRARY})
target_link_libraries(MaxMain ${OPENSSL_LIBRARIES})
target_link_libraries(MaxMain ${PHYSX_LIBRARIES})
target_link_libraries(MaxMain debug ${PYTHON_DEBUG_LIBRARY})
target_link_libraries(MaxMain optimized ${PYTHON_LIBRARY})
target_link_libraries(MaxMain ${Speex_LIBRARY})
target_link_libraries(MaxMain ${Vorbis_LIBRARIES})

# Carbon copy from plClient
# TODO: Maybe see if some of these can be removed?
target_link_libraries(MaxMain CoreLib)
target_link_libraries(MaxMain pfAnimation)
target_link_libraries(MaxMain pfAudio)
target_link_libraries(MaxMain pfCamera)
target_link_libraries(MaxMain pfCCR)
target_link_libraries(MaxMain pfCharacter)
target_link_libraries(MaxMain pfConditional)
target_link_libraries(MaxMain pfConsole)
target_link_libraries(MaxMain pfCsrSrv)
target_link_libraries(MaxMain pfGameGUIMgr)
target_link_libraries(MaxMain pfGameMgr)
target_link_libraries(MaxMain pfGameScoreMgr)
target_link_libraries(MaxMain pfJournalBook)
target_link_libraries(MaxMain pfLocalizationMgr)
target_link_libraries(MaxMain pfMessage)
target_link_libraries(MaxMain pfPython)
target_link_libraries(MaxMain pfSecurePreloader)
target_link_libraries(MaxMain pfStackTrace)
target_link_libraries(MaxMain pfSurface)
target_link_libraries(MaxMain plAgeDescription)
target_link_libraries(MaxMain plAgeLoader)
target_link_libraries(MaxMain plAudible)
target_link_libraries(MaxMain plAudio)
target_link_libraries(MaxMain plAudioCore)
target_link_libraries(MaxMain plAvatar)
target_link_libraries(MaxMain plClientResMgr)
target_link_libraries(MaxMain plCompression)
target_link_libraries(MaxMain plContainer)
target_link_libraries(MaxMain plClientResMgr)
target_link_libraries(MaxMain plDrawable)
target_link_libraries(MaxMain plFile)
target_link_libraries(MaxMain plGImage)
target_link_libraries(MaxMain plGLight)
target_link_libraries(MaxMain plInputCore)
target_link_libraries(MaxMain plInterp)
target_link_libraries(MaxMain plIntersect)
target_link_libraries(MaxMain plJPEG)
target_link_libraries(MaxMain plMath)
target_link_libraries(MaxMain plMessage)
target_link_libraries(MaxMain plModifier)
target_link_libraries(MaxMain plNetClient)
target_link_libraries(MaxMain plNetClientComm)
target_link_libraries(MaxMain plNetClientRecorder)
target_link_libraries(MaxMain plNetCommon)
target_link_libraries(MaxMain plNetGameLib)
target_link_libraries(MaxMain plNetMessage)
target_link_libraries(MaxMain plNetTransport)
target_link_libraries(MaxMain plParticleSystem)
target_link_libraries(MaxMain plPhysical)
target_link_libraries(MaxMain plPhysX)
target_link_libraries(MaxMain plPipeline)
target_link_libraries(MaxMain plProgressMgr)
target_link_libraries(MaxMain plResMgr)
target_link_libraries(MaxMain plScene)
target_link_libraries(MaxMain plSDL)
target_link_libraries(MaxMain plSockets)
target_link_libraries(MaxMain plStatGather)
target_link_libraries(MaxMain plStatusLog)
target_link_libraries(MaxMain plStreamLogger)
target_link_libraries(MaxMain plSurface)
target_link_libraries(MaxMain plTransform)
target_link_libraries(MaxMain plUnifiedTime)
target_link_libraries(MaxMain plVault)
target_link_libraries(MaxMain pnAddrInfo)
target_link_libraries(MaxMain pnAsyncCore)
target_link_libraries(MaxMain pnAsyncCoreExe)
target_link_libraries(MaxMain pnCsrNet)
target_link_libraries(MaxMain pnDispatch)
target_link_libraries(MaxMain pnEncryption)
target_link_libraries(MaxMain pnFactory)
target_link_libraries(MaxMain pnGameMgr)
target_link_libraries(MaxMain pnInputCore)
target_link_libraries(MaxMain pnKeyedObject)
target_link_libraries(MaxMain pnMessage)
target_link_libraries(MaxMain pnModifier)
target_link_libraries(MaxMain pnNetBase)
target_link_libraries(MaxMain pnNetCli)
target_link_libraries(MaxMain pnNetCommon)
target_link_libraries(MaxMain pnNetProtocol)
target_link_libraries(MaxMain pnNucleusInc)
target_link_libraries(MaxMain pnProduct)
target_link_libraries(MaxMain pnSceneObject)
target_link_libraries(MaxMain pnSimpleNet)
target_link_libraries(MaxMain pnTimer)
target_link_libraries(MaxMain pnUtils)
target_link_libraries(MaxMain pnUtilsExe)
target_link_libraries(MaxMain pnUUID)

if (WIN32)
    target_link_libraries(MaxMain Comctl32)
    target_link_libraries(MaxMain Rpcrt4)
    target_link_libraries(MaxMain Shlwapi)
    target_link_libraries(MaxMain Strmiids)
    target_link_libraries(MaxMain Vfw32)
    target_link_libraries(MaxMain Ws2_32)
    target_link_libraries(MaxMain winmm)
endif(WIN32)

source_group("Header Files" FILES ${MaxMain_HEADERS})
source_group("Resource Files" FILES ${MaxMain_RESOURCES})
source_group("Source Files" FILES ${MaxMain_SOURCES})
