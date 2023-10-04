include_directories("${PLASMA_SOURCE_ROOT}/Apps")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

if(PLASMA_BUILD_CLIENT)
    add_subdirectory(plClient)

    if(WIN32)
        add_subdirectory(plCrashHandler)
    endif()
endif()

if(PLASMA_BUILD_LAUNCHER AND WIN32)
    add_subdirectory(plUruLauncher)
endif()

if(PLASMA_BUILD_TOOLS)
    add_subdirectory(SoundDecompress)
endif()
