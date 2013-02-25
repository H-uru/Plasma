include_directories("../../Plasma/CoreLib")

if(WIN32)
    include_directories(${DirectX_INCLUDE_DIR})
endif(WIN32)

set(plShaderAssembler_SOURCES
    main.cpp
)


add_executable(plShaderAssembler ${plShaderAssembler_SOURCES})
target_link_libraries(plShaderAssembler ${DirectX_LIBRARIES})
target_link_libraries(plShaderAssembler CoreLib)

source_group("Source Files" FILES ${plShaderAssembler_SOURCES})
