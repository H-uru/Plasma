include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../FeatureLib")

set(plCrashHandler_SOURCES
    main.cpp
)

add_executable(plCrashHandler ${plCrashHandler_SOURCES})
if(PLASMA_EXTERNAL_RELEASE)
    set_target_properties(plCrashHandler PROPERTIES OUTPUT_NAME "UruCrashHandler")
endif(PLASMA_EXTERNAL_RELEASE)
target_link_libraries(plCrashHandler CoreLib)
target_link_libraries(plCrashHandler pfCrashHandler)
target_link_libraries(plCrashHandler plFile)
target_link_libraries(plCrashHandler pnProduct)
target_link_libraries(plCrashHandler pnUtils)

# Platform specific libs
if(WIN32)
	target_link_libraries(plCrashHandler Dbghelp)
endif(WIN32)
