set(pfGLPipeline_SOURCES
    plGLDevice.cpp
    plGLEnumerate.cpp
    plGLDeviceRefs.cpp
    plGLPipeline.cpp
    plGLPlateManager.cpp
)

set(pfGLPipeline_HEADERS
    plGLDevice.h
    plGLDeviceRef.h
    plGLPipeline.h
    pfGLPipelineCreatable.h
    plGLPlateManager.h
)

plasma_library(pfGLPipeline
    FOLDER FeatureLib
    SOURCES ${pfGLPipeline_SOURCES} ${pfGLPipeline_HEADERS}
)
target_link_libraries(pfGLPipeline
    PUBLIC
        CoreLib
        pnNucleusInc
        plPipeline
    PRIVATE
        plStatusLog
        epoxy::epoxy

        # On macOS, we need to link against OpenGL.framework for access to the
        # CGL functions. On other platforms, the OpenGL library is loaded
        # dynamically by epoxy.
        "$<$<PLATFORM_ID:Darwin>:-framework OpenGL>"
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${pfGLPipeline_SOURCES})
source_group("Header Files" FILES ${pfGLPipeline_HEADERS})
