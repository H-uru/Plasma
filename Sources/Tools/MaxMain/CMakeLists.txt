set(MaxMain_HEADERS
    GlobalUtility.h
    hsMStringTokenizer.h
    main.def
    MaxAPI.h
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
    veroverrides.h
    SimpleExport.rc
    PlasmaMax.gup.manifest
)

set(MaxMain_SOURCES
    GlobalUtility.cpp
    indexes.cpp
    main.cpp
    plActionTableMgr.cpp
    plAgeDescInterface.cpp
    plCommonObjLib.cpp
    plComponentDlg.cpp
    plComponentPanel.cpp
    plGetLocationDlg.cpp
    plMaxAccelerators.cpp
    plMaxCFGFile.cpp
    plMaxFileData.cpp
    plMaxMenu.cpp
    plMaxMeshExtractor.cpp
    plMaxNode.cpp
    plMaxNodeBase.cpp
    plMaxUtils.cpp
    plMtlCollector.cpp
    plNodeLock.cpp
    plPhysicalProps.cpp
    plPluginResManager.cpp
    plPythonMgr.cpp
    plResCollector.cpp
    plResetXform.cpp
    plSaveSelected.cpp
    plTextureExportLog.cpp
    plTextureSearch.cpp
)

plasma_library(MaxMain SHARED
    SOURCES ${MaxMain_HEADERS} ${MaxMain_RESOURCES} ${MaxMain_SOURCES}
    PRECOMPILED_HEADERS Pch.h
    NO_SANITIZE # The 3ds Max headers are full of clang diagnostic errors.
)
target_link_libraries(
    MaxMain
    PUBLIC
        CoreLib
        pnKeyedObject
        pnNucleusInc
        plResMgr
    PRIVATE
        pnFactory
        pnMessage
        pnModifier
        pnNetCommon
        pnSceneObject
        plAgeDescription
        plDrawable
        plFile
        plGImage
        plInterp
        plMessage
        plMessageBox
        plModifier
        plParticleSystem
        plPhysical
        plPhysX
        plPipeline
        plPubUtilInc
        plResMgr
        plScene
        plSDL
        pfAnimation
        pfAudio
        pfCamera
        pfCharacter
        pfConditional
        pfGameMgr
        pfGameGUIMgr
        pfJournalBook
        pfLocalizationMgr
        pfMessage
        pfPython
        pfSurface

        MaxComponent
        MaxConvert
        MaxExport
        MaxPlasmaLights
        MaxPlasmaMtls

        3dsm
        Comctl32
        Python3::Python
)

set_target_properties(MaxMain PROPERTIES OUTPUT_NAME "PlasmaMax")
set_target_properties(MaxMain PROPERTIES SUFFIX ".gup")

install(
    TARGETS MaxMain
    DESTINATION "3dsmax${3dsm_DISPLAY_VERSION}"
)

source_group("Header Files" FILES ${MaxMain_HEADERS})
source_group("Resource Files" FILES ${MaxMain_RESOURCES})
source_group("Source Files" FILES ${MaxMain_SOURCES})
