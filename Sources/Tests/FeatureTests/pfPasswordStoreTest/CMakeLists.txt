set(pfPasswordStoreTest_SOURCES
    test_pfPasswordStore.cpp
)

add_executable(test_pfPasswordStore ${pfPasswordStoreTest_SOURCES})
target_link_libraries(
    test_pfPasswordStore
    PRIVATE
        gtest
        gtest_main
        pnUUID
        pfPasswordStore
)

if(TARGET PkgConfig::LIBSECRET)
    target_compile_definitions(test_pfPasswordStore PRIVATE HAVE_LIBSECRET)
endif()

add_test(NAME test_pfPasswordStore COMMAND test_pfPasswordStore)
add_dependencies(check test_pfPasswordStore)
