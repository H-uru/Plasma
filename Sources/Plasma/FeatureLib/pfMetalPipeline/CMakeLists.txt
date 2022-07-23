include(FetchContent)

FetchContent_Declare(
  metalcpp
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
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
    PRIVATE
        plStatusLog
    INTERFACE
        pnFactory
)
target_include_directories(pfMetalPipeline PUBLIC ${metalcpp_SOURCE_DIR})
target_include_directories(pfMetalPipeline PUBLIC "ShaderSrc")

source_group("Source Files" FILES ${pfMetalPipeline_SOURCES})
source_group("Header Files" FILES ${pfMetalPipeline_HEADERS})
