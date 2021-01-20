cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE STRING "")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CMakeDependentOption)
include(FeatureSummary)
include(Dependencies)
include(VcpkgToolchain)

project(Plasma)

# Set up Product Identification parameters
set(PRODUCT_BRANCH_ID   "1"         CACHE STRING "Branch ID")
set(PRODUCT_BUILD_ID    "918"       CACHE STRING "Build ID")
set(PRODUCT_BUILD_TYPE  "50"        CACHE STRING "Build Type")
set(PRODUCT_CORE_NAME   "UruLive"   CACHE STRING "Product Core Name")
set(PRODUCT_SHORT_NAME  "UruLive"   CACHE STRING "Product Short Name")
set(PRODUCT_LONG_NAME   "Uru Live"  CACHE STRING "Product Long Name")
set(PRODUCT_UUID        "ea489821-6c35-4bd0-9dae-bb17c585e680"
                                    CACHE STRING "Product UUID")

# HeadSpin Configuration
if(WIN32 AND NOT CYGWIN)
    add_definitions(-DHS_BUILD_FOR_WIN32)
endif(WIN32 AND NOT CYGWIN)

if(UNIX)
    # This is set for both Linux and Mac builds
    add_definitions(-DHS_BUILD_FOR_UNIX)
endif(UNIX)

if(APPLE)
    add_definitions(-DHS_BUILD_FOR_APPLE)

    if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
        add_definitions(-DHS_BUILD_FOR_MACOS)
    endif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
endif(APPLE)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(-DHS_BUILD_FOR_LINUX)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
# End HeadSpin Configuration

# Find all 3rd-party libraries that are required
find_package(CURL REQUIRED)
find_package(expat REQUIRED)
find_package(freetype)
find_package(JPEG REQUIRED)
find_package(LIBSECRET)
find_package(Ogg REQUIRED)
find_package(OpenAL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Opus)
find_package(PhysX REQUIRED)
find_package(PNG REQUIRED)
find_package(Python3 3.8 REQUIRED Interpreter Development)
find_package(Security)
find_package(Speex)
find_package(string_theory 2.0 REQUIRED)
include_directories(${STRING_THEORY_INCLUDE_DIRS}) # FIXME: make this less promiscuous
find_package(Vorbis REQUIRED)
find_package(VPX)
find_package(ZLIB REQUIRED)

include(BuildOptimizations)
include(PythonUtils)

if(WIN32)
    # libCurl isn't smart enough to detect this for us, so we have to configure it ourselves
    option(CURL_IS_STATIC "Using the static version of libcurl?" ON)
    if(CURL_IS_STATIC)
        add_definitions(-DCURL_STATICLIB)
    endif(CURL_IS_STATIC)

    # Same story for expat
    option(EXPAT_IS_STATIC "Using the static version of libexpat" ON)
endif(WIN32)

option(PLASMA_EXTERNAL_RELEASE "Is this release intended for the general public?" OFF)
if(PLASMA_EXTERNAL_RELEASE)
    add_definitions(-DPLASMA_EXTERNAL_RELEASE)
endif(PLASMA_EXTERNAL_RELEASE)

if(WIN32)
    set(PLASMA_PIPELINE "DirectX"
                        CACHE STRING "Which graphics backend to use")
else(WIN32)
    set(PLASMA_PIPELINE "OpenGL"
                        CACHE STRING "Which graphics backend to use")
endif(WIN32)
set_property(CACHE PLASMA_PIPELINE PROPERTY STRINGS "DirectX" "OpenGL")

if(PLASMA_PIPELINE STREQUAL "DirectX")
    find_package(DirectX REQUIRED)

    add_definitions(-DPLASMA_PIPELINE_DX)
endif(PLASMA_PIPELINE STREQUAL "DirectX")

if(PLASMA_PIPELINE STREQUAL "OpenGL")
    find_package(OpenGL REQUIRED)
    add_definitions(-DPLASMA_PIPELINE_GL)
endif(PLASMA_PIPELINE STREQUAL "OpenGL")

#Allow us to disable certain parts of the build
option(PLASMA_BUILD_LAUNCHER "Do we want to build plUruLauncher?" ON)
option(PLASMA_BUILD_TOOLS "Do we want to build the Plasma tools?" ON)
option(PLASMA_BUILD_TESTS "Do we want to build the unit tests?" OFF)

cmake_dependent_option(USE_OPUS "Use Opus audio for voice chat" ON "TARGET Opus::opus" OFF)
cmake_dependent_option(USE_SPEEX "Use Speex audio for voice chat" ON "TARGET Speex::speex" OFF)
cmake_dependent_option(USE_VPX "Use VPX for movie support" ON "TARGET VPX::VPX" OFF)
cmake_dependent_option(USE_WEBM "Use WebM movie player" ON "USE_OPUS AND USE_VPX" OFF)

if(WIN32)
    find_package(3dsm)

    option(3dsm_BUILD_PLUGIN "Do we want to build the 3ds Max plugin?" OFF)
    if (3dsm_BUILD_PLUGIN AND 3dsm_FOUND)
        add_definitions(-DBUILDING_MAXPLUGIN)
    endif()
else()
    set(3dsm_BUILD_PLUGIN OFF)
endif()

if(WIN32)
    find_package(VLD)


    option(USE_VLD "Build and link with Visual Leak Detector" OFF)
    if(USE_VLD AND VLD_FOUND)
        add_definitions(-DUSE_VLD)
        include_directories(${VLD_INCLUDE_DIR})
    endif()
endif()

# Configure compile-time compiler-specific flags and other config files
include(cmake/CompilerChecks.cmake)

configure_file("${PROJECT_SOURCE_DIR}/cmake/hsConfig.h.cmake" "${PROJECT_BINARY_DIR}/hsConfig.h")
include_directories(${PROJECT_BINARY_DIR})

if(PLASMA_BUILD_TOOLS)
    # Custom dummy target for compiling all tools
    add_custom_target(tools)
endif()

include(TestBigEndian)
test_big_endian(BIG_ENDIAN)
if(BIG_ENDIAN)
    add_definitions(-DBIG_ENDIAN)
else()
    add_definitions(-DLITTLE_ENDIAN)
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_SCL_SECURE_NO_DEPRECATE)
endif(MSVC)

set(PLASMA_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Sources/Plasma")
set(PLASMA_TOOLS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Sources/Tools")

add_subdirectory(Scripts)
add_subdirectory(Sources/Plasma)
add_subdirectory(Sources/Tools)

if(PLASMA_BUILD_TESTS)
    enable_testing()
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>)
    add_subdirectory(Sources/Tests EXCLUDE_FROM_ALL)
endif(PLASMA_BUILD_TESTS)

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
