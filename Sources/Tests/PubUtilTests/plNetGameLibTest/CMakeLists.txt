set(plNetGameLibTest_SOURCES
    test_plNglAuth.cpp
    test_plNglFile.cpp
)

plasma_test(test_plNetGameLib SOURCES ${plNetGameLibTest_SOURCES})
target_link_libraries(
    test_plNetGameLib
    PRIVATE
        CoreLib
        pnNetProtocol
        plNetGameLib
        gtest_main
)
