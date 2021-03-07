set(plCrashHandler_SOURCES
    main.cpp
)

plasma_executable(plCrashHandler CLIENT SOURCES ${plCrashHandler_SOURCES})
target_link_libraries(
    plCrashHandler
    PRIVATE
        CoreLib
        pfCrashHandler
)

if(PLASMA_EXTERNAL_RELEASE)
    set_target_properties(plCrashHandler PROPERTIES OUTPUT_NAME "UruCrashHandler")
endif(PLASMA_EXTERNAL_RELEASE)
