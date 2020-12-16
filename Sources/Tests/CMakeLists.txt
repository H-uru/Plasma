set(gtest_force_shared_crt ON CACHE INTERNAL
    "don't link the CRT statically into gtest as we later link the test executable dynamically")
add_subdirectory(gtest-1.8.0)

include_directories(${GTEST_INCLUDE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_subdirectory(CoreTests)
add_subdirectory(NucleusTests)
add_subdirectory(PubUtilTests)
add_subdirectory(FeatureTests)
