set(CoreLibTest_SOURCES
    test_endianSwap.cpp
    test_expected.cpp
    test_plCmdParser.cpp
    test_RAMStream.cpp
    $<$<PLATFORM_ID:Darwin>:test_hsDarwin_CF.cpp>
    $<$<PLATFORM_ID:Darwin>:test_hsDarwin_NS.mm>
)

plasma_test(test_CoreLib SOURCES ${CoreLibTest_SOURCES})
target_link_libraries(
    test_CoreLib
    PRIVATE
        CoreLib
        gtest_main
        "$<$<PLATFORM_ID:Darwin>:-framework Foundation>"
)
