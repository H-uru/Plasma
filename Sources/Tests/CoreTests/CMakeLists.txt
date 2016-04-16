include_directories(${GTEST_INCLUDE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../../Plasma/CoreLib)

SET(CoreLibTest_SOURCES
    test_plString.cpp
    test_plFormat.cpp
    test_plCmdParser.cpp
    )

add_executable(test_CoreLib ${CoreLibTest_SOURCES})

target_link_libraries(test_CoreLib gtest gtest_main)
target_link_libraries(test_CoreLib CoreLib)

add_test(NAME test_CoreLib COMMAND test_CoreLib)
add_dependencies(check test_CoreLib)
