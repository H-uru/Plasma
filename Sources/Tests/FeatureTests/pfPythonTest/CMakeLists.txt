include_directories(${GTEST_INCLUDE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../../../Plasma/CoreLib)
include_directories(../../../Plasma/NucleusLib)
include_directories(../../../Plasma/PubUtilLib)
include_directories(../../../Plasma/FeatureLib)

add_executable(test_cyMisc test_cyMisc.cpp)
target_link_libraries(test_cyMisc gtest gtest_main)
target_link_libraries(test_cyMisc pfPython)

if(PYTHON_DEBUG_LIBRARY)
    target_link_libraries(test_cyMisc debug ${PYTHON_DEBUG_LIBRARY})
    target_link_libraries(test_cyMisc optimized ${PYTHON_LIBRARY})
else()
    target_link_libraries(test_cyMisc ${PYTHON_LIBRARY})
endif()

add_test(NAME test_cyMisc COMMAND test_cyMisc)
