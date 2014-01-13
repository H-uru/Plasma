project(CoreLibTests)
 
include_directories(${GTEST_INCLUDE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../../Plasma/CoreLib)
SET(TEST_PLSTRING
PlStringTest.cpp)

add_executable(plStringTest ${TEST_PLSTRING})

target_link_libraries(plStringTest gtest gtest_main CoreLib)

add_test(
NAME plStringTest
COMMAND plStringTest
)
