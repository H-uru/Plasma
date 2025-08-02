set(external_SCRIPTS
    external/create_resource_dat.py
    external/makeres.py
    external/render_svg.py
    external/scalergba.py
)

set(external_SOURCES
    external/Cursor_Base.svg
    external/Linking_Book.svg
    external/Loading_Text_rasterfont.svg
    external/Voice_Chat.svg
)
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
        list(APPEND OPTIMIZE_ARGUMENT --pngcrush "${PNGCRUSH_EXECUTABLE}")
    endif()
    if(RESOURCE_BRUTE)
        list(APPEND OPTIMIZE_ARGUMENT "--brute")
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
)

set(plClient_RESOURCES
)

set(plClient_TEXT
    ${Plasma_SOURCE_DIR}/Docs/ReleaseNotes/ReleaseNotes.txt
)

if(WIN32)
    list(APPEND plClient_SOURCES
        win32/plClient_Win.cpp
        win32/winmain.cpp
    )
    list(APPEND plClient_RESOURCES
        win32/res/plClient.exe.manifest
        win32/res/plClient.rc
        win32/res/resource.h
        win32/res/banner.bmp
        win32/res/cnsl1.bin
        win32/res/Dirt.ICO
        win32/res/headspin.ico
    )
elseif(APPLE)
    list(APPEND plClient_SOURCES
        Mac-Cocoa/main.mm
        Mac-Cocoa/plMacDisplayHelper.mm
        Mac-Cocoa/NSString+StringTheory.mm
        Mac-Cocoa/PLSKeyboardEventMonitor.mm
        Mac-Cocoa/PLSView.mm
        Mac-Cocoa/PLSLoginWindowController.mm
        Mac-Cocoa/PLSPatcherWindowController.mm
        Mac-Cocoa/PLSPatcher.mm
        Mac-Cocoa/PLSServerStatus.mm
    )
    list(APPEND plClient_HEADERS
        Mac-Cocoa/NSString+StringTheory.h
        Mac-Cocoa/plMacDisplayHelper.h
        Mac-Cocoa/PLSKeyboardEventMonitor.h
        Mac-Cocoa/PLSView.h
        Mac-Cocoa/PLSLoginWindowController.h
        Mac-Cocoa/PLSPatcherWindowController.h
        Mac-Cocoa/PLSPatcher.h
        Mac-Cocoa/PLSServerStatus.h
    )
    list(APPEND plClient_RESOURCES
        Mac-Cocoa/banner.png
        Mac-Cocoa/banner@2x.png
        Mac-Cocoa/MainMenu.xib
        Mac-Cocoa/PLSLoginWindowController.xib
        Mac-Cocoa/PLSPatcherWindowController.xib
    )
else()
    list(APPEND plClient_SOURCES
        main.cpp
    )
endif()

plasma_executable(plClient CLIENT INSTALL_PDB
    FOLDER Apps
    SOURCES
        ${plClient_SOURCES} ${plClient_HEADERS}
        ${plClient_TEXT} ${plClient_RESOURCES}
)

if(APPLE)
    # We need to filter out the XIB files so the source XML files don't get bundled
    set(plClient_XCODE_RESOURCES ${plClient_RESOURCES})
    list(FILTER plClient_XCODE_RESOURCES EXCLUDE REGEX "\\.x(ib|cassets)$")

    set_target_properties(plClient PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Mac-Cocoa/Info.plist.in"
        # Hard coding the app name here intentionally.
        # Should prevent macOS from tripping over secured resources being
        # accessed from two different apps. In theory - plClient is just
        # the debug version of UruExplorer. It also prevents invalid characters.
        # GUI identifier should follow the format of a reverse domain string.
        # https://developer.apple.com/library/archive/documentation/CoreFoundation/Conceptual/CFBundles/AccessingaBundlesContents/AccessingaBundlesContents.html
        MACOSX_BUNDLE_GUI_IDENTIFIER org.Huru.UruExplorer
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        RESOURCE "${plClient_XCODE_RESOURCES}"
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
        XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] "YES"
        XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=RelWithDebInfo] "YES"
        XCODE_ATTRIBUTE_INSTALL_PATH "/Applications"
        XCODE_ATTRIBUTE_SKIP_INSTALL "NO"
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/Mac-Cocoa/plClient.entitlements"
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME "YES"
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER org.Huru.UruExplorer
    )
    target_compile_options(plClient PRIVATE -fobjc-arc)
    target_sources(plClient PRIVATE Mac-Cocoa/Assets.xcassets)
    set_source_files_properties(Mac-Cocoa/Assets.xcassets ${RESOURCES} PROPERTIES
         MACOSX_PACKAGE_LOCATION Resources
    )

    if(PLASMA_APPLE_DEVELOPMENT_TEAM_ID)
            set_target_properties(plClient PROPERTIES
                XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
                XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${PLASMA_APPLE_DEVELOPMENT_TEAM_ID}
            )
    endif()
endif()

if(PLASMA_PIPELINE_METAL)
    target_embed_metal_shader_libraries(plClient
        pfMetalPipelineShadersMSL21
        pfMetalPipelineShadersMSL23
    )
endif()

if(PLASMA_BUILD_RESOURCE_DAT)
    target_sources(plClient PRIVATE ${external_SCRIPTS} ${external_SOURCES} ${external_DAT})
endif()

target_link_libraries(
    plClient
    PRIVATE
        CoreLib

        # For the "all creatables"
        pnNucleusInc
        plPubUtilInc
        pfFeatureInc

        # Everything else used in this target.
        pnDispatch
        pnFactory
        pnKeyedObject
        pnMessage
        pnSceneObject
        plAgeLoader
        plAnimation
        plAudio
        plAvatar
        plClientResMgr
        plDrawable
        plFile
        plGImage
        plGLight
        plInputCore
        plMessage
        plMessageBox
        plModifier
        plNetClient
        plNetCommon
        plNetGameLib
        plPhysX
        plPipeline
        plProgressMgr
        plResMgr
        plScene
        plSDL
        plStatGather
        plStatusLog
        plUnifiedTime
        pfAnimation
        pfAudio
        pfCharacter
        pfConsole
        pfConsoleCore
        $<$<PLATFORM_ID:Windows>:pfCrashHandler>
        pfGameGUIMgr
        pfJournalBook
        pfLocalizationMgr
        pfMoviePlayer
        pfPasswordStore
        pfPatcher
        pfPython
        $<$<BOOL:${PLASMA_PIPELINE_DX}>:pfDXPipeline>
        $<$<BOOL:${PLASMA_PIPELINE_GL}>:pfGLPipeline>
        $<$<BOOL:${PLASMA_PIPELINE_METAL}>:pfMetalPipeline>
        CURL::libcurl
        "$<$<PLATFORM_ID:Darwin>:-framework Cocoa>"
        "$<$<PLATFORM_ID:Darwin>:-framework QuartzCore>"
        $<$<PLATFORM_ID:Darwin>:${LibArchive_LIBRARIES}>
)
target_include_directories(plClient PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

if(PLASMA_EXTERNAL_RELEASE)
    set_target_properties(plClient PROPERTIES OUTPUT_NAME "UruExplorer")
endif(PLASMA_EXTERNAL_RELEASE)

if(APPLE)
    # CMake will attempt to strip rpaths, thus preventing fixup_bundle from finding libraries.
    # Not sure why - turning that off.
    set_target_properties(plClient PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
    install(
        CODE
        [[
            include("BundleUtilities")
            fixup_bundle("${CMAKE_INSTALL_PREFIX}/client/$<TARGET_FILE_NAME:plClient>.app" "" "${searchDirs}" IGNORE_ITEM Python)
            execute_process(COMMAND "codesign" "--remove-signature" "--deep" "-f" "${CMAKE_INSTALL_PREFIX}/client/$<TARGET_FILE_NAME:plClient>.app")
        ]]
        DESTINATION client
    )
endif()

if(PLASMA_BUILD_RESOURCE_DAT)
    source_group("Client Resource Scripts" FILES ${external_SCRIPTS})
    source_group("Client Resource Images" FILES ${external_SOURCES})
endif()
source_group("Source Files" FILES ${plClient_SOURCES})
source_group("Header Files" FILES ${plClient_HEADERS})
source_group("Text" FILES ${plClient_TEXT})
source_group("Resources" FILES ${plClient_RESOURCES})
