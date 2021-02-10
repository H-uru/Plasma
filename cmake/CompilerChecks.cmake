# Detect Clang compiler
if("${CMAKE_CXX_COMPILER_ID}" MATCHES ".*Clang")
    set(CMAKE_COMPILER_IS_CLANGXX 1)
endif()

# MSVC automatically defines -D_DEBUG when /MTd or /MDd is set, so we
# need to make sure it gets added for other compilers too
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
endif()

# Use LTCG if available.
cmake_policy(SET CMP0069 NEW)   # gtest projects use old cmake compatibility...
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

# Check for CPUID headers
try_compile(HAVE_CPUID ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_cpuid.cpp)
if(HAVE_CPUID)
    message(STATUS "CPUID header found -- using hardware math acceleration when available")
else()
    message(STATUS "CPUID header not found -- using software math")
endif()
