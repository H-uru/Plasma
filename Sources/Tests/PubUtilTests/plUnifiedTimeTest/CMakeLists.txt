include_directories(${GTEST_INCLUDE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../../../Plasma/CoreLib)
include_directories(../../../Plasma/NucleusLib)
include_directories(../../../Plasma/PubUtilLib)

set(plUnifiedTimeTest_SOURCES
    test_plUnifiedTime.cpp
    )

add_executable(test_plUnifiedTime ${plUnifiedTimeTest_SOURCES})
target_link_libraries(test_plUnifiedTime gtest gtest_main)
target_link_libraries(test_plUnifiedTime plUnifiedTime)
target_link_libraries(test_plUnifiedTime ${STRING_THEORY_LIBRARIES})

add_test(NAME test_plUnifiedTime COMMAND test_plUnifiedTime)
add_dependencies(check test_plUnifiedTime)
