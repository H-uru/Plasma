set(plNetClientTest_SOURCES
    test_plNetClientMsgScreener.cpp
)

plasma_test(test_plNetClient SOURCES ${plNetClientTest_SOURCES})
target_include_directories(test_plNetClient PRIVATE "${PLASMA_SOURCE_ROOT}/FeatureLib")
target_link_libraries(
    test_plNetClient
    PRIVATE
        CoreLib
        pnNucleusInc
        plNetClient
        plPubUtilInc
        pfAnimation
        pfAudio
        pfCamera
        pfConditional
        pfMessage
        gtest_main
)
