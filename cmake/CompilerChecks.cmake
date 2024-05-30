# Use LTCG if available.
if(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _IPO_SUPPORTED LANGUAGES CXX OUTPUT _IPO_OUTPUT)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ${_IPO_SUPPORTED} CACHE BOOL "Use link-time optimizations")
    if(_IPO_SUPPORTED)
        message(STATUS "IPO supported: using link-time optimizations")
    else()
        message(STATUS "IPO not supported: ${_IPO_OUTPUT}")
    endif()
endif()

include(CheckCXXSymbolExists)
# Check for Linux sysinfo.
check_cxx_symbol_exists("sysinfo" "sys/sysinfo.h" HAVE_SYSINFO)
# Check for pthread setname_np API
check_cxx_symbol_exists(pthread_setname_np pthread.h HAVE_PTHREAD_SETNAME_NP)

# Check for BSD style sysctl.
try_compile(HAVE_SYSCTL ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_sysctl.cpp)

# Check for Apple sysdir header.
CHECK_INCLUDE_FILE("sysdir.h" HAVE_SYSDIR)

# Check for `__builtin_available()` Apple extension
try_compile(HAVE_BUILTIN_AVAILABLE ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_builtin_available.cpp)

# Check for CPUID headers
try_compile(HAVE_CPUID ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_cpuid.cpp)

# Check for SIMD headers
CHECK_INCLUDE_FILE("immintrin.h" HAVE_AVX2)
CHECK_INCLUDE_FILE("immintrin.h" HAVE_AVX)
CHECK_INCLUDE_FILE("nmmintrin.h" HAVE_SSE42)
CHECK_INCLUDE_FILE("tmmintrin.h" HAVE_SSSE3)
CHECK_INCLUDE_FILE("smmintrin.h" HAVE_SSE41)
CHECK_INCLUDE_FILE("tmmintrin.h" HAVE_SSE4)
CHECK_INCLUDE_FILE("pmmintrin.h" HAVE_SSE3)
CHECK_INCLUDE_FILE("emmintrin.h" HAVE_SSE2)
CHECK_INCLUDE_FILE("xmmintrin.h" HAVE_SSE1)

# Check for Windows Shell Scaling DPI support
CHECK_INCLUDE_FILE("ShellScalingApi.h" HAVE_SHELLSCALINGAPI)

# GCC requires us to set the -m<instructionset> flag for the source file using the intrinsics.
# We can't do that project-wide or we'll just crash on launch with an illegal instruction on some
# systems. So, we have another helper method...
function(plasma_target_simd_sources TARGET)
    set(_INSTRUCTION_SETS "SSE1;SSE2;SSE3;SSE4;SSE41;SSSE3;SSE42;AVX;AVX2")
    set(_GCC_ARGS "-msse;-msse2;-msse3;-msse4;-msse4.1;-mssse3;-msse4.2;-mavx;-mavx2")
    cmake_parse_arguments(PARSE_ARGV 1 _passf "" "SOURCE_GROUP" "${_INSTRUCTION_SETS}")

    # Hack: if we ever bump to CMake 3.17, use ZIP_LISTS.
    list(LENGTH _INSTRUCTION_SETS _LENGTH)
    math(EXPR _LENGTH "${_LENGTH} - 1")

    foreach(i RANGE ${_LENGTH})
        # foreach(i IN ZIP_LISTS _INSTRUCTION_SETS _GCC_ARGS)
        list(GET _INSTRUCTION_SETS ${i} _CURRENT_INSTRUCTION_SET)
        list(GET _GCC_ARGS ${i} _CURRENT_COMPILE_OPTION)

        set(_CURRENT_SOURCE_FILES ${_passf_${_CURRENT_INSTRUCTION_SET}})
        if(_CURRENT_SOURCE_FILES)
            target_sources(${TARGET} PRIVATE ${_CURRENT_SOURCE_FILES})
            if(_passf_SOURCE_GROUP)
                source_group(${_passf_SOURCE_GROUP} FILES ${_CURRENT_SOURCE_FILES})
            endif()
            if(HAVE_${_CURRENT_INSTRUCTION_SET} AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
                set_property(
                    SOURCE ${_CURRENT_SOURCE_FILES} APPEND PROPERTY
                    COMPILE_OPTIONS ${_CURRENT_COMPILE_OPTION}
                )
                # Can't use the precompiled header for these files due to the changed flags.
                if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
                    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
                        get_target_property(_UNITY_MODE ${TARGET} UNITY_BUILD_MODE)
                        string(COMPARE EQUAL "${_UNITY_MODE}" "BATCH" _SKIP_UNITY)
                    else()
                        set(_SKIP_UNITY TRUE)
                    endif()
                    set_source_files_properties(
                        ${_CURRENT_SOURCE_FILES} PROPERTIES
                        SKIP_PRECOMPILE_HEADERS TRUE
                        SKIP_UNITY_BUILD_INCLUSION ${_SKIP_UNITY}
                    )
                endif()
            endif()
        endif()
    endforeach()
endfunction()
