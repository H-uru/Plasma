include(FetchContent)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

FetchContent_Declare(
    metalcpp
    URL_HASH_SHA256 0afd87ca851465191ae4e3980aa036c7e9e02fe32e7c760ac1a74244aae6023b
    URL "https://developer.apple.com/metal/cpp/files/metal-cpp_macOS13.3_iOS16.4.zip"
)

FetchContent_MakeAvailable(metalcpp)

set(pfMetalPipeline_SOURCES
    plMetalDevice.cpp
    plMetalDeviceRefs.cpp
    plMetalEnumerate.mm
    plMetalMaterialShaderRef.cpp
    plMetalPipeline.cpp
    plMetalPipelineState.cpp
    plMetalPlateManager.cpp
    plMetalShader.cpp
    plMetalFragmentShader.cpp
    plMetalVertexShader.cpp
    plMetalTextFont.cpp
    plMetalDevicePerformanceShaders.mm
    plMetalDeviceLibraryLoader.mm
)

set(pfMetalPipeline_HEADERS
    plMetalDevice.h
    plMetalDeviceRef.h
    plMetalMaterialShaderRef.h
    plMetalPipeline.h
    plMetalPipelineState.h
    plMetalPlateManager.h
    plMetalShader.h
    plMetalTextFont.h
    plMetalFragmentShader.h
    plMetalVertexShader.h
    ShaderSrc/ShaderTypes.h
    ShaderSrc/ShaderVertex.h
    pfMetalPipelineCreatable.h
)

set(pfMetalPipeline_SHADERS
    ShaderSrc/FixedPipelineShaders.metal
    ShaderSrc/PlateShaders.metal
    ShaderSrc/BiasNormals.metal
    ShaderSrc/CompCosines.metal
    ShaderSrc/WaveSet7.metal
    ShaderSrc/Grass.metal
    ShaderSrc/WaveDecEnv.metal
    ShaderSrc/Avatar.metal
    ShaderSrc/WaveDec1Lay_7.metal
    ShaderSrc/WaveRip.metal
    ShaderSrc/Clear.metal
    ShaderSrc/GammaCorrection.metal
    ShaderSrc/TextFontShader.metal
)

plasma_library(pfMetalPipeline
    FOLDER FeatureLib
    SOURCES ${pfMetalPipeline_SOURCES} ${pfMetalPipeline_HEADERS}
)

target_link_libraries(pfMetalPipeline
    PUBLIC
        CoreLib
        pnNucleusInc
        plPipeline
        Metal::Metal
        "-framework MetalPerformanceShaders"
    PRIVATE
        plStatusLog
    INTERFACE
        pnFactory
)
add_dependencies(
    pfMetalPipeline
    pfMetalPipelineShadersMSL21
    pfMetalPipelineShadersMSL23
)

target_include_directories(pfMetalPipeline PUBLIC ${metalcpp_SOURCE_DIR})
target_include_directories(pfMetalPipeline PUBLIC "ShaderSrc")

source_group("Source Files" FILES ${pfMetalPipeline_SOURCES})
source_group("Header Files" FILES ${pfMetalPipeline_HEADERS})

# All supported Macs for Plasma/Metal support MSL 2.1
add_metal_shader_library(pfMetalPipelineShadersMSL21
    STANDARD macos-metal2.1
    ${pfMetalPipeline_SHADERS}
)
target_include_directories(pfMetalPipelineShadersMSL21
    PRIVATE
    $<TARGET_PROPERTY:pnNucleusInc,INTERFACE_INCLUDE_DIRECTORIES>
)
set_target_properties(pfMetalPipelineShadersMSL21 PROPERTIES
    FOLDER FeatureLib
)

# Build a 2.3 varient for macOS 11 and/or Apple Silicon specific features
add_metal_shader_library(pfMetalPipelineShadersMSL23
    STANDARD macos-metal2.3
    ${pfMetalPipeline_SHADERS}
)
target_include_directories(pfMetalPipelineShadersMSL23
    PRIVATE
    $<TARGET_PROPERTY:pnNucleusInc,INTERFACE_INCLUDE_DIRECTORIES>
)
set_target_properties(pfMetalPipelineShadersMSL23 PROPERTIES
    FOLDER FeatureLib
)
