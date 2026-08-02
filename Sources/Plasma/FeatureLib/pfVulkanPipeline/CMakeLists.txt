set(pfVulkanPipeline_SOURCES
    plVulkanDevice.cpp
    plVulkanEnumerate.cpp
    plVulkanPipeline.cpp
    plVulkanPlateManager.cpp
    plVulkanShaders.cpp
    plVulkanDeviceRefs.cpp
    plVulkanMemory.cpp
    plVulkanBuffers.cpp
    plVulkanTextures.cpp
    plVulkanPipelineState.cpp
    plVulkanPipelineCache.cpp
    plVulkanDescriptors.cpp
    plVulkanDraw.cpp
    plVulkanTextFont.cpp
    plVulkanLights.cpp
    plVulkanMaterial.cpp
    plVulkanAuxSpans.cpp
    plVulkanAvatar.cpp
    plVulkanProjections.cpp
    plVulkanShaderPath.cpp
    plVulkanShadows.cpp
    plVulkanMaterialShaderRef.cpp
)

set(pfVulkanPipeline_HEADERS
    pfVulkanPipelineCreatable.h
    plVulkanDevice.h
    plVulkanMaterialShaderRef.h
    plVulkanShaderRef.h
    plVulkanPipeline.h
    plVulkanPlateManager.h
    plVulkanShaders.h
    plVulkanDeviceRef.h
    plVulkanPipelineState.h
    plVulkanTextFont.h
)

set(pfVulkanPipeline_SHADERS
    ShaderSrc/FixedPipeline.vert
    ShaderSrc/FixedPipeline.frag
    ShaderSrc/Plate.vert
    ShaderSrc/Plate.frag
    ShaderSrc/TextFont.vert
    ShaderSrc/TextFont.frag
    ShaderSrc/ShadowCaster.vert
    ShaderSrc/ShadowCaster.frag
    ShaderSrc/ShadowApply.vert
    ShaderSrc/ShadowApply.frag
    ShaderSrc/Avatar.vert
    ShaderSrc/Avatar.frag
    ShaderSrc/Fullscreen.vert
    ShaderSrc/Blur.frag
    ShaderSrc/Gamma.frag
    ShaderSrc/Grass.vert
    ShaderSrc/Grass.frag
    ShaderSrc/CompCosines.vert
    ShaderSrc/CompCosines.frag
    ShaderSrc/BiasNormals.vert
    ShaderSrc/BiasNormals.frag
    ShaderSrc/WaveRip.vert
    ShaderSrc/WaveRip.frag
    ShaderSrc/WaveDec1Lay_7.vert
    ShaderSrc/CbaseAbase.frag
    ShaderSrc/WaveDecEnv.vert
    ShaderSrc/WaveDecEnv.frag
    ShaderSrc/WaveSet7.vert
    ShaderSrc/WaveSet7.frag
)

# Anything a shader includes; touching one of these has to recompile every shader.
set(pfVulkanPipeline_SHADER_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/ShaderSrc/plVulkanShaderTypes.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ShaderSrc/FixedFunctionVertex.glsl"
    "${CMAKE_CURRENT_SOURCE_DIR}/ShaderSrc/WaveConsts.glsl"
    "${PLASMA_SOURCE_ROOT}/NucleusLib/inc/hsGMatStateEnums.h"
)

# Compile each shader to a C header of SPIR-V words, so the bytecode is linked
# into the library. Nothing to install, nothing to locate at runtime. This is the
# same idea as the .metallib the Metal backend embeds, minus the bundle.
set(pfVulkanPipeline_SPIRV)
foreach(_shader IN LISTS pfVulkanPipeline_SHADERS)
    get_filename_component(_shader_name "${_shader}" NAME)
    set(_spirv_header "${CMAKE_CURRENT_BINARY_DIR}/ShaderSrc/${_shader_name}.spv.h")

    add_custom_command(
        OUTPUT "${_spirv_header}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/ShaderSrc"
        # glslangValidator requires -I to be glued to the path, no space.
        COMMAND "${GLSLANG_VALIDATOR_EXECUTABLE}"
                -V --target-env vulkan1.3
                "-I${CMAKE_CURRENT_SOURCE_DIR}/ShaderSrc"
                "-I${PLASMA_SOURCE_ROOT}/NucleusLib/inc"
                -x -o "${_spirv_header}"
                "${CMAKE_CURRENT_SOURCE_DIR}/${_shader}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_shader}" ${pfVulkanPipeline_SHADER_HEADERS}
        COMMENT "Compiling ${_shader_name} to SPIR-V"
        VERBATIM
    )

    list(APPEND pfVulkanPipeline_SPIRV "${_spirv_header}")
endforeach()

plasma_library(pfVulkanPipeline
    FOLDER FeatureLib
    SOURCES
        ${pfVulkanPipeline_SOURCES} ${pfVulkanPipeline_HEADERS}
        ${pfVulkanPipeline_SHADERS} ${pfVulkanPipeline_SPIRV}
)
set_source_files_properties(${pfVulkanPipeline_SHADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
target_include_directories(pfVulkanPipeline PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

# volk resolves Vulkan entry points at runtime, so VMA must not assume static
# linkage. Dynamic fetching lets it derive the whole table from the two loader
# pointers we hand it; with both disabled it demands every pointer up front.
target_compile_definitions(pfVulkanPipeline
    PRIVATE
        VMA_STATIC_VULKAN_FUNCTIONS=0
        VMA_DYNAMIC_VULKAN_FUNCTIONS=1
)
target_link_libraries(pfVulkanPipeline
    PUBLIC
        CoreLib
        pnNucleusInc
        plPipeline

        # plVulkanDevice holds Vulkan handles by value and pl3DPipeline holds the
        # device by value, so consumers of plVulkanPipeline.h need the headers.
        # volk defines VK_NO_PROTOTYPES itself; nothing links a Vulkan loader.
        volk::volk_headers
    PRIVATE
        plStatusLog
        pfDisplayHelpers

        # volk_headers above only declares the entry point table; this is what
        # defines it and provides volkInitialize/volkLoadDevice.
        volk::volk

        GPUOpen::VulkanMemoryAllocator

        # SDL owns the window, and SDL_Vulkan_CreateSurface is the only thing
        # that gets the surface right across X11, Wayland and everything else
        # SDL supports. When a non-SDL frontend needs this backend, replace that
        # one call with a native handle descriptor.
        SDL3::SDL3
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${pfVulkanPipeline_SOURCES})
source_group("Header Files" FILES ${pfVulkanPipeline_HEADERS})
source_group("Shaders" FILES ${pfVulkanPipeline_SHADERS})
