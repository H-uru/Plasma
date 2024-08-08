set(pnUUIDTest_SOURCES
    test_plUUID.cpp
)

plasma_test(test_pnUUID SOURCES ${pnUUIDTest_SOURCES})
target_link_libraries(
    test_pnUUID
    PRIVATE
        CoreLib
        pnUUID
        gtest_main
)

