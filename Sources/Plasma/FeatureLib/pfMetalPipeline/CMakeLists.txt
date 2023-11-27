include(FetchContent)

FetchContent_Declare(
    metalcpp
    URL_HASH_SHA256 0afd87ca851465191ae4e3980aa036c7e9e02fe32e7c760ac1a74244aae6023b
    URL "https://developer.apple.com/metal/cpp/files/metal-cpp_macOS13.3_iOS16.4.zip"
)

FetchContent_MakeAvailable(metalcpp)

set(pfMetalPipeline_SOURCES
    plMetalDevice.cpp
    plMetalDeviceRefs.cpp
    plMetalMaterialShaderRef.cpp
    plMetalPipeline.cpp
    plMetalPipelineState.cpp
    plMetalPlateManager.cpp
    plMetalShader.cpp
    plMetalFragmentShader.cpp
    plMetalVertexShader.cpp
    plMetalTextFont.cpp
    plMetalEnumerate.mm
    plMetalDevicePerformanceShaders.mm
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

plasma_library(pfMetalPipeline SOURCES ${pfMetalPipeline_SOURCES} ${pfMetalPipeline_HEADERS})
target_link_libraries(pfMetalPipeline
    PUBLIC
        CoreLib
        pnNucleusInc
        plPipeline
        "-framework Metal"
        "-framework MetalPerformanceShaders"
    PRIVATE
        plStatusLog
    INTERFACE
        pnFactory
)

target_include_directories(pfMetalPipeline PUBLIC ${metalcpp_SOURCE_DIR})
target_include_directories(pfMetalPipeline PUBLIC "ShaderSrc")

source_group("Source Files" FILES ${pfMetalPipeline_SOURCES})
source_group("Header Files" FILES ${pfMetalPipeline_HEADERS})

add_library(pfMetalPipelineShaders INTERFACE)
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
set_source_files_properties(${pfMetalPipeline_SHADERS} TARGET_DIRECTORY plClient PROPERTIES LANGUAGE METAL)
# source group does not work with an interface library in Xcode, but maybe someday...
source_group("Metal Shaders" FILES ${pfMetalPipeline_SHADERS})

target_sources(pfMetalPipelineShaders INTERFACE ${pfMetalPipeline_SHADERS})
