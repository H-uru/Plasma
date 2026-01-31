set(hsG3DDeviceDumper_SOURCES
    plAllCreatables.cpp
    main.cpp
)

plasma_executable(hsG3DDeviceDumper TOOL
    FOLDER Tools
    SOURCES ${hsG3DDeviceDumper_SOURCES}
)

target_link_libraries(
    hsG3DDeviceDumper
    PRIVATE
        CoreLib
        plPipeline
        pfDisplayHelpers

        # For the "all creatables"
        pnNucleusInc
        plPubUtilInc
        pfFeatureInc

        # Everything else used in this target.
        $<$<BOOL:${PLASMA_PIPELINE_GL}>:pfGLPipeline>
        $<$<BOOL:${PLASMA_PIPELINE_METAL}>:pfMetalPipeline>
)

source_group("Source Files" FILES ${hsG3DDeviceDumper_SOURCES})
