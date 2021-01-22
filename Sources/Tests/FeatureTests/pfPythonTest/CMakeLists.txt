set(pfPythonTest_SOURCES
    test_cyMisc.cpp
)

add_executable(test_pfPython ${pfPythonTest_SOURCES})
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

add_test(NAME test_pfPython COMMAND test_pfPython)
add_dependencies(check test_pfPython)
