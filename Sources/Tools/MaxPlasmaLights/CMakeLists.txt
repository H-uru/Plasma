include_directories("../")
include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/PubUtilLib")
include_directories("../../Plasma/PubUtilLib/inc")
include_directories(${3dsm_INCLUDE_DIR})

set(MaxPlasmaLights_HEADERS
    plRealTimeLightBase.h
    plRealTimeLights.h
    plRealTimeLightsPBDec.h
    plRTObjLightDesc.h
    plRTPlasmaLights.def
    plRTProjDirLight.h
    plRTProjDirLightClassDesc.h
    plRTProjDirLightPBDec.h
)

set(MaxPlasmaLights_RESOURCES
    plRTLights.rc
    resource.h
)


set(MaxPlasmaLights_SOURCES
    DLLEntry.cpp
    plRealTimeLightBase.cpp
    plRealTimeLights.cpp
    plRTProjDirLight.cpp
)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Plasma_BINARY_DIR}/bin)
use_precompiled_header(Pch.h Pch.cpp MaxPlasmaLights_HEADERS MaxPlasmaLights_SOURCES)
add_library(MaxPlasmaLights SHARED ${MaxPlasmaLights_HEADERS} ${MaxPlasmaLights_RESOURCES} ${MaxPlasmaLights_SOURCES})
set_target_properties(MaxPlasmaLights PROPERTIES SUFFIX ".dlo")


target_link_libraries(MaxPlasmaLights MaxComponent)
target_link_libraries(MaxPlasmaLights MaxPlasmaMtls)
target_link_libraries(MaxPlasmaLights ${3dsm_LIBRARIES})

# Carbon copy from plClient
# TODO: Maybe see if some of these can be removed?
target_link_libraries(MaxPlasmaLights CoreLib)
target_link_libraries(MaxPlasmaLights pfAnimation)
target_link_libraries(MaxPlasmaLights pfAudio)
target_link_libraries(MaxPlasmaLights pfCamera)
target_link_libraries(MaxPlasmaLights pfCCR)
target_link_libraries(MaxPlasmaLights pfCharacter)
target_link_libraries(MaxPlasmaLights pfConditional)
target_link_libraries(MaxPlasmaLights pfConsole)
target_link_libraries(MaxPlasmaLights pfGameGUIMgr)
target_link_libraries(MaxPlasmaLights pfGameMgr)
target_link_libraries(MaxPlasmaLights pfGameScoreMgr)
target_link_libraries(MaxPlasmaLights pfJournalBook)
target_link_libraries(MaxPlasmaLights pfLocalizationMgr)
target_link_libraries(MaxPlasmaLights pfMessage)
target_link_libraries(MaxPlasmaLights pfPython)
target_link_libraries(MaxPlasmaLights pfSecurePreloader)
target_link_libraries(MaxPlasmaLights pfSurface)
target_link_libraries(MaxPlasmaLights plAgeDescription)
target_link_libraries(MaxPlasmaLights plAgeLoader)
target_link_libraries(MaxPlasmaLights plAudible)
target_link_libraries(MaxPlasmaLights plAudio)
target_link_libraries(MaxPlasmaLights plAudioCore)
target_link_libraries(MaxPlasmaLights plAvatar)
target_link_libraries(MaxPlasmaLights plCompression)
target_link_libraries(MaxPlasmaLights plContainer)
target_link_libraries(MaxPlasmaLights plDrawable)
target_link_libraries(MaxPlasmaLights plFile)
target_link_libraries(MaxPlasmaLights plGImage)
target_link_libraries(MaxPlasmaLights plGLight)
target_link_libraries(MaxPlasmaLights plInputCore)
target_link_libraries(MaxPlasmaLights plInterp)
target_link_libraries(MaxPlasmaLights plIntersect)
target_link_libraries(MaxPlasmaLights plMath)
target_link_libraries(MaxPlasmaLights plMessage)
target_link_libraries(MaxPlasmaLights plModifier)
target_link_libraries(MaxPlasmaLights plNetClient)
target_link_libraries(MaxPlasmaLights plNetClientComm)
target_link_libraries(MaxPlasmaLights plNetClientRecorder)
target_link_libraries(MaxPlasmaLights plNetCommon)
target_link_libraries(MaxPlasmaLights plNetGameLib)
target_link_libraries(MaxPlasmaLights plNetMessage)
target_link_libraries(MaxPlasmaLights plNetTransport)
target_link_libraries(MaxPlasmaLights plParticleSystem)
target_link_libraries(MaxPlasmaLights plPhysical)
target_link_libraries(MaxPlasmaLights plPhysX)
target_link_libraries(MaxPlasmaLights plPipeline)
target_link_libraries(MaxPlasmaLights plProgressMgr)
target_link_libraries(MaxPlasmaLights plResMgr)
target_link_libraries(MaxPlasmaLights plScene)
target_link_libraries(MaxPlasmaLights plSDL)
target_link_libraries(MaxPlasmaLights plSockets)
target_link_libraries(MaxPlasmaLights plStatGather)
target_link_libraries(MaxPlasmaLights plStatusLog)
target_link_libraries(MaxPlasmaLights plStreamLogger)
target_link_libraries(MaxPlasmaLights plSurface)
target_link_libraries(MaxPlasmaLights plTransform)
target_link_libraries(MaxPlasmaLights plUnifiedTime)
target_link_libraries(MaxPlasmaLights plVault)
target_link_libraries(MaxPlasmaLights pnAsyncCore)
target_link_libraries(MaxPlasmaLights pnAsyncCoreExe)
target_link_libraries(MaxPlasmaLights pnDispatch)
target_link_libraries(MaxPlasmaLights pnEncryption)
target_link_libraries(MaxPlasmaLights pnFactory)
target_link_libraries(MaxPlasmaLights pnGameMgr)
target_link_libraries(MaxPlasmaLights pnInputCore)
target_link_libraries(MaxPlasmaLights pnKeyedObject)
target_link_libraries(MaxPlasmaLights pnMessage)
target_link_libraries(MaxPlasmaLights pnModifier)
target_link_libraries(MaxPlasmaLights pnNetBase)
target_link_libraries(MaxPlasmaLights pnNetCli)
target_link_libraries(MaxPlasmaLights pnNetCommon)
target_link_libraries(MaxPlasmaLights pnNucleusInc)
target_link_libraries(MaxPlasmaLights pnSceneObject)
target_link_libraries(MaxPlasmaLights pnTimer)
target_link_libraries(MaxPlasmaLights pnUtils)
target_link_libraries(MaxPlasmaLights pnUUID)

if (WIN32)
    target_link_libraries(MaxPlasmaLights Comctl32)
    target_link_libraries(MaxPlasmaLights Rpcrt4)
    target_link_libraries(MaxPlasmaLights Ws2_32)
endif(WIN32)

if(USE_VLD)
    target_link_libraries(MaxPlasmaLights ${VLD_LIBRARY})
endif()

source_group("Header Files" FILES ${MaxPlasmaLights_HEADERS})
source_group("Resource Files" FILES ${MaxPlasmaLights_RESOURCES})
source_group("Source Files" FILES ${MaxPlasmaLights_SOURCES})
