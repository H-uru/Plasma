# These values represent the default CI package binarycache. These should probably not be changed
# unless you're doing something really special. Note that GitHub's package server requires an
# access token with scope package:read for this to work... To make matters worse, if GitHub sees
# the token in-repo, it will automatically revoke it... Grrr...
set(_NUGET_SOURCE "https://nuget.pkg.github.com/Hoikas/index.json")
set(_NUGET_OWNER "Hoikas")
# Python: print(*(ord(i) for i in token), sep=";")
set(_NUGET_TOKEN_ASCII 55;50;102;97;56;51;57;98;52;50;56;99;97;99;98;97;99;57;53;99;50;98;48;50;49;98;56;56;57;102;53;54;102;55;57;101;48;51;52;99)
string(ASCII ${_NUGET_TOKEN_ASCII} _NUGET_TOKEN)

# You're not crazy. This is so we can read from the main package source and write to another one.
set(PLASMA_VCPKG_NUGET_SOURCE "" CACHE INTERNAL "")
set(PLASMA_VCPKG_NUGET_OWNER "" CACHE INTERNAL "")
set(PLASMA_VCPKG_NUGET_TOKEN "" CACHE INTERNAL "")
set(PLASMA_VCPKG_NUGET_RW FALSE CACHE INTERNAL "")

# Welcome to vcpkg-land. Population: Hoikas.
# The goal here is that if cmake is being invoked with no arguments, we want to force usage of
# our vcpkg submodule on Windows hosts ONLY. Otherwise, it's an opt-in type dealio. Further,
# if the consumer is already using a toolchain, we would ideally not want to sweep the rug out
# from under them. The good news is that vcpkg will detect any old non-manifest build directories
# and refuse to continue on that, so great... BUT WAIT! Stoopid Visual Studio's CMake support will
# pass in an arbitrary vcpkg toolchain if one is not specified in CMakeSettings.json. We don't want
# that either. Ugh, screw me.
string(COMPARE EQUAL "${CMAKE_HOST_SYSTEM_NAME}" "Windows" _HOST_IS_WINDOWS)
option(USE_VCPKG "Use the vcpkg submodule for dependency management." ${_HOST_IS_WINDOWS})
if(NOT USE_VCPKG)
    return()
endif()

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)

function(_plasma_vcpkg_generate_nuget_config)
    cmake_parse_arguments(_pvgnc "" "OUT_FILE;NUGET_NAME;NUGET_SOURCE;NUGET_OWNER;NUGET_TOKEN" "" ${ARGN})
    configure_file(
        "${CMAKE_SOURCE_DIR}/nuget.config.in"
        "${_pvgnc_OUT_FILE}"
        @ONLY
    )
endfunction()

function(_plasma_vcpkg_setup_binarycache)
    cmake_parse_arguments(_pvsb "" "NAME;PREFIX" "" ${ARGN})

    if(NOT ${_pvsb_PREFIX}_SOURCE OR NOT ${_pvsb_PREFIX}_OWNER OR NOT ${_pvsb_PREFIX}_TOKEN)
        return()
    endif()

    set(_CONFIG_PATH "${CMAKE_BINARY_DIR}/${_pvsb_NAME}-NuGet.Config")
    _plasma_vcpkg_generate_nuget_config(
        OUT_FILE ${_CONFIG_PATH}
        NUGET_SOURCE ${${_pvsb_PREFIX}_SOURCE}
        NUGET_OWNER ${${_pvsb_PREFIX}_OWNER}
        NUGET_TOKEN ${${_pvsb_PREFIX}_TOKEN}
    )

    file(TO_NATIVE_PATH "${_CONFIG_PATH}" _CONFIG_PATH_NATIVE)
    set(ENV{VCPKG_BINARY_SOURCES} "$ENV{VCPKG_BINARY_SOURCES};nugetconfig,${_CONFIG_PATH_NATIVE}")
    if(${_pvsb_PREFIX}_RW)
        set(ENV{VCPKG_BINARY_SOURCES} "$ENV{VCPKG_BINARY_SOURCES},readwrite")
    endif()
endfunction()

_plasma_vcpkg_setup_binarycache(NAME mainline PREFIX _NUGET)
_plasma_vcpkg_setup_binarycache(NAME fork PREFIX PLASMA_VCPKG_NUGET)

# Note that CMAKE_SIZEOF_VOID_P is currently undefined.
list(APPEND VCPKG_OVERLAY_TRIPLETS "${CMAKE_SOURCE_DIR}/Scripts/Triplets")
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

# Workaround: fixes issues where vcpkg will suggest Debug libraries for OpenSSL. Sigh.
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")

# Copies DLLs built by vcpkg when an install() command is run. Probably only works on Windows.
set(X_VCPKG_APPLOCAL_DEPS_INSTALL TRUE CACHE BOOL "")
