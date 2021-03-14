set(pfPythonTest_SOURCES
    test_cyMisc.cpp
)

plasma_test(test_pfPython SOURCES ${pfPythonTest_SOURCES})
target_link_libraries(
    test_pfPython
    PRIVATE
        CoreLib
        pnNucleusInc
        plPubUtilInc
        pfFeatureInc
        pfPython
        gtest_main
)
