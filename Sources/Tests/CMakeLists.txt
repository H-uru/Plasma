set(gtest_force_shared_crt ON CACHE INTERNAL
    "don't link the CRT statically into gtest as we later link the test executable dynamically")
add_subdirectory(gtest-1.7.0)

add_subdirectory(CoreTests)
add_subdirectory(NucleusTests)
add_subdirectory(PubUtilTests)
add_subdirectory(FeatureTests)
