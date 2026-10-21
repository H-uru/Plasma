set(pfGameGUIMgrTest_SOURCES
    test_pfGUIMultiLineEditCtrl.cpp
)

plasma_test(test_pfGameGUIMgr SOURCES ${pfGameGUIMgrTest_SOURCES})
target_link_libraries(
    test_pfGameGUIMgr
    PRIVATE
        gtest
        gtest_main
        CoreLib
        pnNucleusInc
        pfFeatureInc
        plPubUtilInc
        pfGameGUIMgr
)
