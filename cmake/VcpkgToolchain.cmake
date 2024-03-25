# These values represent the default CI package binarycache. These should probably not be changed
# unless you're doing something really special. Note that GitHub's package server requires an
# access token with scope package:read for this to work... To make matters worse, if GitHub sees
# the token in-repo, it will automatically revoke it... Grrr...
set(PLASMA_NUGET_SOURCE "https://nuget.pkg.github.com/H-uru/index.json")
set(PLASMA_NUGET_OWNER "H-uruMachineUser")
# Python: print(*(ord(i) for i in token), sep=";")
set(_PLASMA_NUGET_TOKEN_ASCII 103;104;112;95;100;111;87;99;122;56;49;97;76;101;110;122;82;116;119;112;80;49;97;87;72;107;71;57;103;51;110;100;100;112;52;69;57;88;73;48)
string(ASCII ${_PLASMA_NUGET_TOKEN_ASCII} PLASMA_NUGET_TOKEN)

# Welcome to vcpkg-land. Population: Hoikas.
# The goal here is that if cmake is being invoked with no arguments, we want to force usage of
# our vcpkg submodule on Windows hosts ONLY. Otherwise, it's an opt-in type dealio. Further,
# if the consumer is already using a toolchain, we would ideally not want to sweep the rug out
# from under them. The good news is that vcpkg will detect any old non-manifest build directories
# and refuse to continue on that, so great... BUT WAIT! Stoopid Visual Studio's CMake support will
# pass in an arbitrary vcpkg toolchain if one is not specified in CMakeSettings.json. We don't want
# that either. Ugh, screw me.
string(COMPARE EQUAL "${CMAKE_HOST_SYSTEM_NAME}" "Windows" _HOST_IS_WINDOWS)
string(COMPARE EQUAL "${CMAKE_HOST_SYSTEM_NAME}" "Darwin" _HOST_IS_DARWIN)
option(USE_VCPKG "Use the vcpkg submodule for dependency management." ${_HOST_IS_WINDOWS})
if(NOT USE_VCPKG)
    return()
endif()

include(SubmoduleUtils)
plasma_init_git_submodule("vcpkg" RESULT has_vcpkg_module)
if(NOT has_vcpkg_module)
    return()
endif()

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)

# Binarycache can only be used on Windows or if mono is available.
find_program(_VCPKG_MONO mono)
if(_HOST_IS_WINDOWS OR EXISTS "${_VCPKG_MONO}")
    set(_NUGET_CONFIG_PATH "${CMAKE_BINARY_DIR}/NuGet.Config")
    configure_file(
        "${CMAKE_SOURCE_DIR}/NuGet.Config.in"
        "${_NUGET_CONFIG_PATH}"
        @ONLY
    )

    file(TO_NATIVE_PATH "${_NUGET_CONFIG_PATH}" _NUGET_CONFIG_PATH_NATIVE)
    set(ENV{VCPKG_BINARY_SOURCES} "$ENV{VCPKG_BINARY_SOURCES};nugetconfig,${_NUGET_CONFIG_PATH_NATIVE},read")
endif()

# Note that CMAKE_SIZEOF_VOID_P is currently undefined.
if(_HOST_IS_WINDOWS AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(CMAKE_GENERATOR_PLATFORM MATCHES "[Ww][Ii][Nn]32")
        set(VCPKG_TARGET_TRIPLET "x86-windows-plasma" CACHE STRING "")
    elseif(CMAKE_GENERATOR_PLATFORM MATCHES "[Xx]64")
        set(VCPKG_TARGET_TRIPLET "x64-windows-plasma" CACHE STRING "")
    elseif(NOT CMAKE_GENERATOR_PLATFORM)
        # This is probably the Ninja generator. Unfortunately, we can't try_compile() anything
        # right now, so we'll just steal part of the vcpkg toolchain to maybe guess the triplet.
        find_program(_VCPKG_CL cl)
        if(_VCPKG_CL MATCHES "amd64/cl.exe$" OR _VCPKG_CL MATCHES "x64/cl.exe$")
            set(VCPKG_TARGET_TRIPLET "x64-windows-plasma" CACHE STRING "")
        elseif(_VCPKG_CL MATCHES "bin/cl.exe$" OR _VCPKG_CL MATCHES "x86/cl.exe$")
            set(VCPKG_TARGET_TRIPLET "x86-windows-plasma" CACHE STRING "")
        else()
            message(FATAL_ERROR "Unknown compiler: '${_VCPKG_CL}' - set VCPKG_TARGET_TRIPLET manually.")
        endif()
    else()
        message(FATAL_ERROR "Unknown platform: '${CMAKE_GENERATOR_PLATFORM}' - set VCPKG_TARGET_TRIPLET manually.")
    endif()
endif()

if(_HOST_IS_DARWIN AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
        set(VCPKG_TARGET_TRIPLET "x64-macos-plasma" CACHE STRING "")
    elseif(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        set(VCPKG_TARGET_TRIPLET "arm64-macos-plasma" CACHE STRING "")
    elseif(DEFINED CMAKE_OSX_ARCHITECTURES)
        set(VCPKG_TARGET_TRIPLET "universal-macos-plasma" CACHE STRING "")
    endif()

    # CMAKE_HOST_SYSTEM_PROCESSOR is undefined at this point
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE _HOST_SYSTEM_PROCESSOR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(VCPKG_HOST_TRIPLET "x64-macos-plasma" CACHE STRING "")
        set(VCPKG_TARGET_TRIPLET "x64-macos-plasma" CACHE STRING "")
    elseif(_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(VCPKG_HOST_TRIPLET "arm64-macos-plasma" CACHE STRING "")
        set(VCPKG_TARGET_TRIPLET "arm64-macos-plasma" CACHE STRING "")
    else()
        message(FATAL_ERROR "Unknown architecture: '${_HOST_SYSTEM_PROCESSOR}' - set VCPKG_TARGET_TRIPLET manually.")
    endif()
endif()

# Default to using the same triplet for the host and target to prevent unnecessary
# dependencies on cross-compilers when we're just building a Win32 static engine
# on a Win64 machine.
if(_HOST_IS_WINDOWS AND NOT DEFINED VCPKG_HOST_TRIPLET)
    set(VCPKG_HOST_TRIPLET ${VCPKG_TARGET_TRIPLET} CACHE STRING "")
endif()

# Workaround: The cairocffi Python module does not ship with libcairo-2.dll as expected. So,
# we're going to opt-into a special manifest feature to install the vcpkg cairo.dll into
# the python3 port. BUT only if PLASMA_BUILD_RESOURCE_DAT has not been explicitly turned OFF
# on the command line (reminder: it may be forced OFF on subsequent cmake runs). This is a feature
# because it's an overlay port that exists in our repository.
if(NOT DEFINED PLASMA_BUILD_RESOURCE_DAT OR PLASMA_BUILD_RESOURCE_DAT)
    list(APPEND VCPKG_MANIFEST_FEATURES cairosvg)
endif()

# Workaround: fixes issues where vcpkg will suggest Debug libraries for OpenSSL. Sigh.
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")

# Copies DLLs built by vcpkg when an install() command is run. Probably only works on Windows.
set(X_VCPKG_APPLOCAL_DEPS_INSTALL TRUE CACHE BOOL "")
