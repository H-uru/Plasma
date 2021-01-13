find_program(
    PNGCRUSH_EXECUTABLE pngcrush
    DOC "Path to pngcrush"
)

# Can we generate the resource.dat file?
python_test_modules(
    MODULES cairosvg PIL
    REQUIREMENTS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/external/requirements.txt"
)

cmake_dependent_option(PLASMA_BUILD_RESOURCE_DAT "Do we want to build resource.dat?" ON "cairosvg_FOUND AND PIL_FOUND" OFF)
cmake_dependent_option(RESOURCE_OPTIMIZE "Optimize the images in resource.dat" ON "PLASMA_BUILD_RESOURCE_DAT AND PNGCRUSH_EXECUTABLE" OFF)
cmake_dependent_option(RESOURCE_BRUTE "Allow pngcrush brute-force optimization" OFF "PLASMA_BUILD_RESOURCE_DAT AND PNGCRUSH_EXECUTABLE" OFF)

if(PLASMA_BUILD_RESOURCE_DAT)
    set(external_DAT "${CMAKE_CURRENT_BINARY_DIR}/resource.dat")
    if(RESOURCE_OPTIMIZE)
        string(APPEND OPTIMIZE_ARGUMENT "--pngcrush \"${PNGCRUSH_EXECUTABLE}\" ")
    endif()
    if(RESOURCE_BRUTE)
        string(APPEND OPTIMIZE_ARGUMENT "--brute ")
    endif()

    add_custom_command(
        OUTPUT ${external_DAT}
        COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/external/makeres.py"
                --render --package
                ${OPTIMIZE_ARGUMENT}
                -i "${CMAKE_CURRENT_SOURCE_DIR}/external"
                -w "${CMAKE_CURRENT_BINARY_DIR}"
                -o "${CMAKE_CURRENT_BINARY_DIR}"
        DEPENDS ${external_SCRIPTS} ${external_SOURCES}
    )

    install(
        FILES "${external_DAT}"
        DESTINATION client
    )
endif()

set(plClient_HEADERS
    plClient.h
    plClientCreatable.h
    plClientLoader.h
    plClientUpdateFormat.h
)

set(plClient_SOURCES
    pfAllCreatables.cpp
    plAllCreatables.cpp
    plClient.cpp
    plClientLoader.cpp
    pnAllCreatables.cpp
    winmain.cpp
)

set(plClient_TEXT
    ${Plasma_SOURCE_DIR}/Docs/ReleaseNotes/ReleaseNotes.txt
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
                              ${plClient_TEXT} ${plClient_RESOURCES}
                              ${external_SCRIPTS} ${external_SOURCES} ${external_DAT}
)

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
target_link_libraries(plClient pfCrashHandler)
target_link_libraries(plClient pfGameGUIMgr)
target_link_libraries(plClient pfGameScoreMgr)
target_link_libraries(plClient pfJournalBook)
target_link_libraries(plClient pfLocalizationMgr)
target_link_libraries(plClient pfMessage)
target_link_libraries(plClient pfMoviePlayer)
target_link_libraries(plClient pfPasswordStore)
target_link_libraries(plClient pfPython)
target_link_libraries(plClient pfSurface)
target_link_libraries(plClient plAgeDescription)
target_link_libraries(plClient plAgeLoader)
target_link_libraries(plClient plAnimation)
target_link_libraries(plClient plAudible)
target_link_libraries(plClient plAudio)
target_link_libraries(plClient plAudioCore)
target_link_libraries(plClient plAvatar)
target_link_libraries(plClient plClientResMgr)
target_link_libraries(plClient plClipboard)
#target_link_libraries(plClient plClientKey)
target_link_libraries(plClient plCompression)
target_link_libraries(plClient plContainer)
target_link_libraries(plClient plDrawable)
target_link_libraries(plClient plFile)
target_link_libraries(plClient plGImage)
target_link_libraries(plClient plGLight)
target_link_libraries(plClient plInputCore)
target_link_libraries(plClient plInterp)
target_link_libraries(plClient plIntersect)
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
target_link_libraries(plClient pnAsyncCore)
target_link_libraries(plClient pnAsyncCoreExe)
target_link_libraries(plClient pnDispatch)
target_link_libraries(plClient pnEncryption)
target_link_libraries(plClient pnFactory)
target_link_libraries(plClient pnInputCore)
target_link_libraries(plClient pnKeyedObject)
target_link_libraries(plClient pnMessage)
target_link_libraries(plClient pnModifier)
target_link_libraries(plClient pnNetBase)
target_link_libraries(plClient pnNetCli)
target_link_libraries(plClient pnNetCommon)
target_link_libraries(plClient pnNetProtocol)
target_link_libraries(plClient pnNucleusInc)
target_link_libraries(plClient pnSceneObject)
target_link_libraries(plClient pnTimer)
target_link_libraries(plClient pnUtils)
target_link_libraries(plClient pnUUID)

target_link_libraries(plClient ${DirectX_LIBRARIES})
target_link_libraries(plClient CURL::libcurl)
target_link_libraries(plClient ${STRING_THEORY_LIBRARIES})

if(USE_VLD)
    target_link_libraries(plClient ${VLD_LIBRARY})
endif()

if (WIN32)
    target_link_libraries(plClient rpcrt4)
    target_link_libraries(plClient version)
    target_link_libraries(plClient vfw32)
    target_link_libraries(plClient ws2_32)
    target_link_libraries(plClient winmm)
    target_link_libraries(plClient strmiids)
    target_link_libraries(plClient crypt32)
    target_link_libraries(plClient wldap32)
endif(WIN32)

install(
    TARGETS plClient
    DESTINATION client
)

install(
    FILES $<TARGET_PDB_FILE:plClient>
    DESTINATION client
    OPTIONAL
)

if(PLASMA_BUILD_RESOURCE_DAT)
    source_group("Client Resource Scripts" FILES ${external_SCRIPTS})
    source_group("Client Resource Images" FILES ${external_SOURCES})
endif()
source_group("Source Files" FILES ${plClient_SOURCES})
source_group("Header Files" FILES ${plClient_HEADERS})
source_group("Text" FILES ${plClient_TEXT})
source_group("Windows Resource Files" FILES ${plClient_RESOURCES})
