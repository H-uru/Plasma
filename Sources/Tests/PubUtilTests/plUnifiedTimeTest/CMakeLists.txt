set(plUnifiedTimeTest_SOURCES
    test_plUnifiedTime.cpp
)

add_executable(test_plUnifiedTime ${plUnifiedTimeTest_SOURCES})
target_link_libraries(
    test_plUnifiedTime
    PRIVATE
        CoreLib
        plUnifiedTime
        gtest_main
)

add_test(NAME test_plUnifiedTime COMMAND test_plUnifiedTime)
add_dependencies(check test_plUnifiedTime)
