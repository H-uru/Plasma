set(pnEncryptionTest_SOURCES
    test_plMD5Checksum.cpp
    test_plSHAChecksum.cpp
    test_plSHA1Checksum.cpp
)

add_executable(test_pnEncryption ${pnEncryptionTest_SOURCES})
target_link_libraries(
    test_pnEncryption
    PRIVATE
        CoreLib
        pnEncryption
        gtest_main
)

add_test(NAME test_pnEncryption COMMAND test_pnEncryption)
add_dependencies(check test_pnEncryption)
