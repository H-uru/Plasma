cmake_dependent_option(
    PLASMA_USE_PCH
    "Enable precompiled headers?"
    ON
    [[${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16"]]
    OFF
)

function(use_precompiled_header TARGET)
    if(PLASMA_USE_PCH)
        target_precompile_headers(${TARGET} PRIVATE ${ARGN})
    endif(PLASMA_USE_PCH)
endfunction(use_precompiled_header)
