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

file(COPY xLoading_Linking.000.png
    xLoading_Linking.001.png
    xLoading_Linking.002.png
    xLoading_Linking.003.png
    xLoading_Linking.004.png
    xLoading_Linking.005.png
    xLoading_Linking.006.png
    xLoading_Linking.007.png
    xLoading_Linking.008.png
    xLoading_Linking.009.png
    xLoading_Linking.010.png
    xLoading_Linking.011.png
    xLoading_Linking.012.png
    xLoading_Linking.013.png
    xLoading_Linking.014.png
    xLoading_Linking.015.png
    xLoading_Linking.016.png
    xLoading_Linking.017.png
    xLoading_Linking.018.png
    xLoading_Linking.019.png
    xLoading_Linking.020.png
    xLoading_Linking.021.png
    xLoading_Linking.022.png
    xLoading_Linking.023.png
    xLoading_Linking.024.png
    xLoading_Linking.025.png
    xLoading_Linking.026.png
    xLoading_Linking.027.png
    xLoading_Linking.028.png
    xLoading_Linking.029.png
    xLoading_Linking.030.png
    xLoading_Linking.031.png
    xLoading_Updating_Text.png
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/render
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
