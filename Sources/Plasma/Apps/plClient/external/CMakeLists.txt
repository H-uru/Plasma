set(external_SCRIPTS
    create_resource_dat.py
    makeres.py
    render_svg.py
    scalergba.py
)

set(external_SOURCES
    Cursor_Base.svg
    Linking_Book.svg
    Loading_Text_rasterfont.svg
    Voice_Chat.svg
)

find_program(
    PNGCRUSH_EXECUTABLE pngcrush
    DOC "Path to pngcrush"
)

include(CMakeDependentOption)
cmake_dependent_option(RESOURCE_OPTIMIZE "Optimize the images in resource.dat" ON PNGCRUSH_EXECUTABLE OFF)
cmake_dependent_option(RESOURCE_BRUTE "Allow pngcrush brute-force optimization" OFF PNGCRUSH_EXECUTABLE OFF)

if(RESOURCE_OPTIMIZE)
    string(APPEND OPTIMIZE_ARGUMENT "--pngcrush \"${PNGCRUSH_EXECUTABLE}\" ")
endif()
if(RESOURCE_BRUTE)
    string(APPEND OPTIMIZE_ARGUMENT "--brute ")
endif()

set(Make_Resource_Command
    ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/makeres.py"
    ${OPTIMIZE_ARGUMENT}
    --render --package
    -i "${CMAKE_CURRENT_SOURCE_DIR}"
    -w "${CMAKE_CURRENT_BINARY_DIR}"
    -o "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

add_custom_command(
    OUTPUT resource.dat
    COMMAND ${Make_Resource_Command}
    DEPENDS ${external_SOURCES} ${external_SCRIPTS}
)
add_custom_target(externalResources 
    SOURCES ${external_SOURCES} ${external_SCRIPTS} 
    DEPENDS resource.dat
)

source_group("Source Files" FILES ${external_SOURCES})
source_group("Script Files" FILES ${external_SCRIPTS})
