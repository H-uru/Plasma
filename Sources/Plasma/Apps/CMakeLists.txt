include_directories("${PLASMA_SOURCE_ROOT}/Apps")
include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib/inc")

if(WIN32)
    add_subdirectory(plClient)
    add_subdirectory(plCrashHandler)

    if(PLASMA_BUILD_LAUNCHER)
        add_subdirectory(plUruLauncher)
    endif()
endif()

if(PLASMA_BUILD_TOOLS)
    add_subdirectory(plPythonPack)
    add_subdirectory(plFileSecure)
    add_subdirectory(plFileEncrypt)
    add_subdirectory(plPageInfo)
    add_subdirectory(plPageOptimizer)

    add_dependencies(tools
        plPythonPack
        plFileSecure
        plFileEncrypt
        plPageInfo
        plPageOptimizer
    )
endif()
