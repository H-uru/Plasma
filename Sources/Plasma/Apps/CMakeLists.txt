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
