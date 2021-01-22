set(CoreLibTest_SOURCES
    test_plCmdParser.cpp
)

add_executable(test_CoreLib ${CoreLibTest_SOURCES})
target_link_libraries(
    test_CoreLib
    PRIVATE
        CoreLib
        gtest_main
)

add_test(NAME test_CoreLib COMMAND test_CoreLib)
add_dependencies(check test_CoreLib)
