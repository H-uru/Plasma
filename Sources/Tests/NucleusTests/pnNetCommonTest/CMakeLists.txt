set(pnNetCommonTest_SOURCES
    test_plNetAddress.cpp
)

plasma_test(test_pnNetCommon SOURCES ${pnNetCommonTest_SOURCES})
target_link_libraries(
    test_pnNetCommon
    PRIVATE
        CoreLib
        pnNetCommon
        gtest_main
)
