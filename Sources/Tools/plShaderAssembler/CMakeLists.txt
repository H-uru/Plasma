set(plShaderAssembler_SOURCES
    main.cpp
)

plasma_executable(plShaderAssembler TOOL
    FOLDER Tools
    SOURCES ${plShaderAssembler_SOURCES}
)
target_link_libraries(
    plShaderAssembler
    PRIVATE
        CoreLib
        ${DirectX_LIBRARIES}
)

source_group("Source Files" FILES ${plShaderAssembler_SOURCES})
