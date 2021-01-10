cmake_dependent_option(
    PLASMA_USE_PCH
    "Enable precompiled headers?"
    ON
    [[${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16"]]
    OFF
)

cmake_dependent_option(
    PLASMA_UNITY_BUILD
    "Enable unity build?"
    ON
    [[${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16"]]
    OFF
)

function(plasma_optimize_target TARGET)
    cmake_parse_arguments(_pot "UNITY_BUILD" "" "PRECOMPILED_HEADERS" ${ARGN})

    if(PLASMA_USE_PCH AND _pot_PRECOMPILED_HEADERS)
        target_precompile_headers(${TARGET} PRIVATE ${_pot_PRECOMPILED_HEADERS})
    endif()

    if(PLASMA_UNITY_BUILD AND _pot_UNITY_BUILD)
        set_target_properties(${TARGET} PROPERTIES UNITY_BUILD TRUE)
    endif()
endfunction()
