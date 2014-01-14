include_directories(${GTEST_INCLUDE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../../../Plasma/CoreLib)
include_directories(../../../Plasma/NucleusLib)

set(pnEncryptionTest_SOURCES
    test_plMD5Checksum.cpp
    test_plSHA1Checksum.cpp
    )

add_executable(test_pnEncryption ${pnEncryptionTest_SOURCES})
target_link_libraries(test_pnEncryption gtest gtest_main)
target_link_libraries(test_pnEncryption pnEncryption)

add_test(NAME test_pnEncryption COMMAND test_pnEncryption)
add_dependencies(check test_pnEncryption)
