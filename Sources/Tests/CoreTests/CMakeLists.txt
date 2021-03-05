set(CoreLibTest_SOURCES
    test_plCmdParser.cpp
)

plasma_test(test_CoreLib SOURCES ${CoreLibTest_SOURCES})
target_link_libraries(
    test_CoreLib
    PRIVATE
        CoreLib
        gtest_main
)
