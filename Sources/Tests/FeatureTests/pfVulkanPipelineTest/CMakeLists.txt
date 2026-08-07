set(pfVulkanPipelineTest_SOURCES
    test_plGTAO.cpp
    test_plVulkanBufferPoolRef.cpp
)

plasma_test(test_pfVulkanPipeline SOURCES ${pfVulkanPipelineTest_SOURCES})
target_link_libraries(
    test_pfVulkanPipeline
    PRIVATE
        CoreLib
        gtest_main
        volk::volk_headers
)
