set(plShaderAssembler_SOURCES
    main.cpp
)

add_executable(plShaderAssembler ${plShaderAssembler_SOURCES})
target_link_libraries(
    plShaderAssembler
    PRIVATE
        CoreLib
        ${DirectX_LIBRARIES}
        string_theory
)

source_group("Source Files" FILES ${plShaderAssembler_SOURCES})
