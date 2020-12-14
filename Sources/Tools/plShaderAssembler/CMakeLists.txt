set(plShaderAssembler_SOURCES
    main.cpp
)

add_executable(plShaderAssembler ${plShaderAssembler_SOURCES})
target_link_libraries(plShaderAssembler ${DirectX_LIBRARIES})
target_link_libraries(plShaderAssembler CoreLib)
target_link_libraries(plShaderAssembler ${STRING_THEORY_LIBRARIES})
if(USE_VLD)
    target_link_libraries(plShaderAssembler ${VLD_LIBRARIES})
endif()

install(
    TARGETS plShaderAssembler
    DESTINATION tools_cli
)

source_group("Source Files" FILES ${plShaderAssembler_SOURCES})
