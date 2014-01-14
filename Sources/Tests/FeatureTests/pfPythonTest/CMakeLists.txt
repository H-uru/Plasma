include_directories(${GTEST_INCLUDE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../../../Plasma/CoreLib)
include_directories(../../../Plasma/NucleusLib)
include_directories(../../../Plasma/PubUtilLib)
include_directories(../../../Plasma/FeatureLib)

set(pfPythonTest_SOURCES
    test_cyMisc.cpp
    )

add_executable(test_pfPython ${pfPythonTest_SOURCES})
target_link_libraries(test_pfPython gtest gtest_main)
target_link_libraries(test_pfPython pfPython)

if(PYTHON_DEBUG_LIBRARY)
    target_link_libraries(test_pfPython debug ${PYTHON_DEBUG_LIBRARY})
    target_link_libraries(test_pfPython optimized ${PYTHON_LIBRARY})
else()
    target_link_libraries(test_pfPython ${PYTHON_LIBRARY})
endif()

add_test(NAME test_pfPython COMMAND test_pfPython)
add_dependencies(check test_pfPython)
