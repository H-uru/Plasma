if(PLASMA_BUILD_TOOLS)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Widgets REQUIRED)

    add_subdirectory(plLocalizationEditor)
    add_subdirectory(plResBrowser)

    add_dependencies(tools
        plLocalizationEditor
        plResBrowser
    )

    if(WIN32)
        add_subdirectory(plShaderAssembler)
        add_dependencies(tools plShaderAssembler)
    endif()

    if(FREETYPE_FOUND)
        add_subdirectory(plFontConverter)
        add_dependencies(tools plFontConverter)
    endif()
endif()

# Max Stuff goes below here...
if(3dsm_BUILD_PLUGIN)
    add_subdirectory(MaxComponent)
    add_subdirectory(MaxConvert)
    add_subdirectory(MaxExport)
    add_subdirectory(MaxMain)
    add_subdirectory(MaxPlasmaLights)
    add_subdirectory(MaxPlasmaMtls)
endif()
