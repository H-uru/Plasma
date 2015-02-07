include_directories(${GTEST_INCLUDE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../../../Plasma/CoreLib)
include_directories(../../../Plasma/NucleusLib)

set(pnUtilsTest_SOURCES
    test_plCmdParser.cpp
    )

add_executable(test_pnUtils ${pnUtilsTest_SOURCES})
target_link_libraries(test_pnUtils gtest gtest_main)
target_link_libraries(test_pnUtils pnUtils)

add_test(NAME test_pnUtils COMMAND test_pnUtils)
add_dependencies(check test_pnUtils)

