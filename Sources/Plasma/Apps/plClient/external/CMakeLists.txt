set(external_SCRIPTS
    makeres.py
	render_svg.py
	create_resource_dat.py
)

set(external_SOURCES
    Cursor_Base.svg
    Linking_Book.svg
    Loading_Text_rasterfont.svg
    Voice_Chat.svg
)

if(PLASMA_EXTERNAL_RELEASE)
    set(Make_Resource_Command
		${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/makeres.py --optimize --render --package -i ${CMAKE_CURRENT_SOURCE_DIR} -w ${CMAKE_CURRENT_BINARY_DIR} -o ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else(PLASMA_EXTERNAL_RELEASE)
	set(Make_Resource_Command
		${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/makeres.py --render --package -i ${CMAKE_CURRENT_SOURCE_DIR} -w ${CMAKE_CURRENT_BINARY_DIR} -o ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif(PLASMA_EXTERNAL_RELEASE)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource.dat
    COMMAND ${Make_Resource_Command}
    DEPENDS ${external_SOURCES} ${external_SCRIPTS}
)
add_custom_target(externalResources DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/resource.dat)

source_group("Source Files" FILES ${external_SOURCES})
source_group("Script Files" FILES ${external_SCRIPTS})
