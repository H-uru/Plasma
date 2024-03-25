set(pnEncryptionTest_SOURCES
    test_plMD5Checksum.cpp
    test_plSHAChecksum.cpp
    test_plSHA1Checksum.cpp
)

plasma_test(test_pnEncryption SOURCES ${pnEncryptionTest_SOURCES})
target_link_libraries(
    test_pnEncryption
    PRIVATE
        CoreLib
        pnEncryption
        gtest_main
)
