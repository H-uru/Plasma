set(pfConsoleCore_SOURCES
    test_pfConsoleCore.cpp
)

plasma_test(test_pfConsoleCore SOURCES ${pfConsoleCore_SOURCES})
target_link_libraries(
    test_pfConsoleCore
    PRIVATE
        CoreLib
        pfConsoleCore
        gtest_main
)
