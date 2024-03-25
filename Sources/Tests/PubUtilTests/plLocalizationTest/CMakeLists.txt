set(plLocalizationTest_SOURCES
    test_plLocalization.cpp
)

plasma_test(test_plLocalization SOURCES ${plLocalizationTest_SOURCES})
target_link_libraries(
    test_plLocalization
    PRIVATE
        CoreLib
        plResMgr
        gtest_main
)
