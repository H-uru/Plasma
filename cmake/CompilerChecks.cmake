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


# Check for CPUID headers
try_compile(HAVE_CPUID ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_cpuid.cpp)
if(HAVE_CPUID)
    message(STATUS "CPUID header found -- using hardware math acceleration when available")
else()
    message(STATUS "CPUID header not found -- using software math")
endif()

# Look for a supported "deprecated" attribute specifier.
try_compile(HAVE_CXX14_DEPRECATED_ATTR ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_deprecated_attribute.cpp
            COMPILE_DEFINITIONS -DTRY_ATTRIBUTE)
try_compile(HAVE_GCC_DEPRECATED_ATTR ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_deprecated_attribute.cpp
            COMPILE_DEFINITIONS -DTRY_GCC_ATTR)

# Look for C++11 constexpr support
try_compile(HAVE_CONSTEXPR ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_constexpr.cpp)

# Look for C++11 override/final specifiers
try_compile(HAVE_OVERRIDE ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_override.cpp)

# Look for C++11 noexcept specifier
try_compile(HAVE_NOEXCEPT ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/cmake/check_noexcept.cpp)
