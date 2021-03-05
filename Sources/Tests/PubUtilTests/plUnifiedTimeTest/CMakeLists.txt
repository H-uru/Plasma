set(plUnifiedTimeTest_SOURCES
    test_plUnifiedTime.cpp
)

plasma_test(test_plUnifiedTime SOURCES ${plUnifiedTimeTest_SOURCES})
target_link_libraries(
    test_plUnifiedTime
    PRIVATE
        CoreLib
        plUnifiedTime
        gtest_main
)
