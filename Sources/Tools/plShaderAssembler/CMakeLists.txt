include_directories("../../Plasma/CoreLib")

if(WIN32)
    # NOTE: Includes of Windows SDK must be used BEFORE includes of DirectX SDK.
    #       Otherwise, it can make error about PVOID64!
    get_filename_component(WindowsSdkDir "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows;CurrentInstallFolder]" ABSOLUTE CACHE)
    include_directories("${WindowsSdkDir}/include" ${DirectX_INCLUDE_DIR})
endif(WIN32)

set(plShaderAssembler_SOURCES
    main.cpp
)


add_executable(plShaderAssembler ${plShaderAssembler_SOURCES})
target_link_libraries(plShaderAssembler ${DirectX_LIBRARIES})
target_link_libraries(plShaderAssembler CoreLib)

source_group("Source Files" FILES ${plShaderAssembler_SOURCES})
