set(pfGLPipeline_SOURCES
    plGLDevice.cpp
    plGLDeviceRefs.cpp
    plGLPipeline.cpp
)

set(pfGLPipeline_HEADERS
    plGLDevice.h
    plGLDeviceRef.h
    plGLPipeline.h
    pfGLPipelineCreatable.h
)

plasma_library(pfGLPipeline SOURCES ${pfGLPipeline_SOURCES} ${pfGLPipeline_HEADERS})
target_link_libraries(pfGLPipeline
    PUBLIC
        CoreLib
        pnNucleusInc
        plPipeline
    PRIVATE
        plStatusLog
        epoxy::epoxy
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${pfGLPipeline_SOURCES})
source_group("Header Files" FILES ${pfGLPipeline_HEADERS})
