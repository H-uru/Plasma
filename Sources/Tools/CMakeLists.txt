find_package(Qt5 COMPONENTS Core Widgets)
set_package_properties(Qt5Core PROPERTIES
    URL "http://www.qt.io/"
    DESCRIPTION "Cross-platform GUI framework for C++"
    TYPE OPTIONAL
    PURPOSE "Required for building GUI tools for Plasma"
)

include_directories("${PLASMA_TOOLS_ROOT}")
include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib/inc")

if(PLASMA_BUILD_TOOLS)
    add_subdirectory(SoundDecompress)
    add_dependencies(tools SoundDecompress)

    if(Qt5_FOUND)
        add_subdirectory(plLocalizationEditor)
        add_subdirectory(plResBrowser)

        add_dependencies(tools
            plLocalizationEditor
            plResBrowser
        )
    endif()

    if(WIN32)
        add_subdirectory(plShaderAssembler)
        add_dependencies(tools plShaderAssembler)
    endif()

    if(FREETYPE_FOUND AND Qt5_FOUND)
        add_subdirectory(plFontConverter)
        add_dependencies(tools plFontConverter)
    endif()
endif()

# Max Stuff goes below here...
if(3dsm_FOUND AND 3dsm_BUILD_PLUGIN)
    add_subdirectory(MaxComponent)
    add_subdirectory(MaxConvert)
    add_subdirectory(MaxExport)
    add_subdirectory(MaxMain)
    add_subdirectory(MaxPlasmaLights)
    add_subdirectory(MaxPlasmaMtls)
endif()
