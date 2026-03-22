set(pfPasswordStoreTest_SOURCES
    test_pfPasswordStore.cpp
)

plasma_test(test_pfPasswordStore SOURCES ${pfPasswordStoreTest_SOURCES})
target_link_libraries(
    test_pfPasswordStore
    PRIVATE
        gtest
        gtest_main
        pnNetBase
        pnUUID
        pfPasswordStore
)

if(TARGET PkgConfig::LIBSECRET)
    target_compile_definitions(test_pfPasswordStore PRIVATE HAVE_LIBSECRET)
endif()
