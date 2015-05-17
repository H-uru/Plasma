# Detect Clang compiler
if("${CMAKE_CXX_COMPILER_ID}" MATCHES ".*Clang")
    set(CMAKE_COMPILER_IS_CLANGXX 1)
endif()

# Require C++11
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
    if(APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif()

# MSVC automatically defines -D_DEBUG when /MTd or /MDd is set, so we
# need to make sure it gets added for other compilers too
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
endif()


# Compile-time type size checks
include(CheckTypeSize)

if(NOT WCHAR_BYTES)
    check_type_size("wchar_t" WCHAR_BYTES)
    if(NOT WCHAR_BYTES)
        message(FATAL_ERROR "Could not determine sizeof(wchar_t)")
        set(WCHAR_BYTES 0)
    endif()
endif()

if(NOT SIZEOF_LONG)
    check_type_size("long" SIZEOF_LONG)
    if(NOT SIZEOF_LONG)
        message(FATAL_ERROR "Could not determine sizeof(long)")
        set(SIZEOF_LONG 0)
    endif()
endif()

# Check for CPUID headers
try_compile(HAVE_CPUID ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_cpuid.cpp
            OUTPUT_VARIABLE OUTPUT)
if(HAVE_CPUID)
    message("CPUID header found -- using hardware math acceleration when available")
else()
    message("CPUID header not found -- using software math")
endif()

# Look for a supported "deprecated" attribute specifier.
try_compile(HAVE_CXX14_DEPRECATED_ATTR ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_deprecated_attribute.cpp
            COMPILE_DEFINITIONS -DTRY_ATTRIBUTE
            OUTPUT_VARIABLE OUTPUT)
try_compile(HAVE_GCC_DEPRECATED_ATTR ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_deprecated_attribute.cpp
            COMPILE_DEFINITIONS -DTRY_GCC_ATTR
            OUTPUT_VARIABLE OUTPUT)

# Look for C++11 constexpr support
try_compile(HAVE_CONSTEXPR ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_constexpr.cpp
            OUTPUT_VARIABLE OUTPUT)

configure_file(${PROJECT_SOURCE_DIR}/cmake/hsCompilerSpecific.h.cmake
               ${PROJECT_BINARY_DIR}/hsCompilerSpecific.h)
include_directories(${PROJECT_BINARY_DIR})
