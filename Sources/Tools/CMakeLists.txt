#add_subdirectory(Migration)
#add_subdirectory(plDatMerger)
#add_subdirectory(plFontConverter)
#add_subdirectory(plResBrowser)

# Max Stuff goes below here...
option(3dsm_BUILD_PLUGIN "Do we want to build the 3ds Max plugin?" OFF)

if(3dsm_BUILD_PLUGIN)
    add_definitions(-DMAXPLUGINCODE)
    
    add_subdirectory(MaxComponent)
    add_subdirectory(MaxConvert)
    add_subdirectory(MaxExport)
    add_subdirectory(MaxMain)
    #add_subdirectory(MaxMtlUpdate)
    add_subdirectory(MaxPlasmaLights)
    add_subdirectory(MaxPlasmaMtls)
    #add_subdirectory(MaxSceneViewer)
endif()