include_directories("../../Plasma/CoreLib")

if(WIN32)
    include_directories(${DirectX_INCLUDE_DIR})
endif(WIN32)

set(AssShader_SOURCES
    main.cpp
)


add_executable(AssShader ${AssShader_SOURCES})
target_link_libraries(AssShader ${DirectX_LIBRARIES})
target_link_libraries(AssShader CoreLib)

source_group("Source Files" FILES ${AssShader_SOURCES})
