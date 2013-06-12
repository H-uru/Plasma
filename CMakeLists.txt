project(Plasma)
cmake_minimum_required(VERSION 2.8)

# Set up Product Identification parameters
set(PRODUCT_BRANCH_ID   "1"         CACHE STRING "Branch ID")
set(PRODUCT_BUILD_ID    "918"       CACHE STRING "Build ID")
set(PRODUCT_BUILD_TYPE  "50"        CACHE STRING "Build Type")
set(PRODUCT_CORE_NAME   "UruLive"   CACHE STRING "Product Core Name")
set(PRODUCT_SHORT_NAME  "UruLive"   CACHE STRING "Product Short Name")
set(PRODUCT_LONG_NAME   "Uru Live"  CACHE STRING "Product Long Name")
set(PRODUCT_UUID        "ea489821-6c35-4bd0-9dae-bb17c585e680"
                                    CACHE STRING "Product UUID")

# Detect Clang compiler
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_CLANGXX 1)
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

# Require C++11
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

# HeadSpin Configuration
if(WIN32 AND NOT CYGWIN)
    add_definitions(-DHS_BUILD_FOR_WIN32)
endif(WIN32 AND NOT CYGWIN)
if(UNIX)
    add_definitions(-DHS_BUILD_FOR_UNIX)
endif(UNIX)
# End HeadSpin Configuration

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Plasma_BINARY_DIR}/bin)

# Find all 3rd-party libraries that are required
find_package(OpenSSL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(EXPAT REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(Ogg REQUIRED)    #TODO: Not required if we aren't building the client
find_package(Vorbis REQUIRED) #TODO: Not required if we aren't building the client
find_package(Speex REQUIRED)  #TODO: Not required if we aren't building the client
find_package(CURL REQUIRED)
find_package(PCRE REQUIRED)

if(WIN32)
    find_package(PhysX REQUIRED)  #TODO: Not required if we aren't building the client
    find_package(DirectX REQUIRED)
    find_package(Freetype)
endif(WIN32)

find_package(MaxSDK)          #TODO: Only find this if we are building PlasmaMax
find_package(Bink)            #TODO: Find Bink, but don't require it if plPipeline isn't built...
#                                    Or better yet, just eliminate bink altogether
include(PrecompiledHeader)    #Precompiled Header helper macros

# libCurl isn't smart enough to detect this for us, so we have to configure it ourselves
option(CURL_IS_STATIC "Using the static version of libcurl?" ON)
if(CURL_IS_STATIC)
    add_definitions(-DCURL_STATICLIB)
endif(CURL_IS_STATIC)

#libpcre isn't smart enough to detect this either
option(PCRE_IS_STATIC "Using the static version of libpcre?" ON)
if(PCRE_IS_STATIC)
    add_definitions(-DPCRE_STATIC)
endif(PCRE_IS_STATIC)

option(PLASMA_EXTERNAL_RELEASE "Is this release intended for the general public?" OFF)
if(PLASMA_EXTERNAL_RELEASE)
    add_definitions(-DPLASMA_EXTERNAL_RELEASE)
endif(PLASMA_EXTERNAL_RELEASE)

set(PLASMA_TARGETS "Client"
    CACHE STRING "Which set of plasma targets to build and use")
set_property(CACHE PLASMA_TARGETS PROPERTY STRINGS
             "Client" "Server" "Patcher" "Ethereal" "NoAvMsgs")

if(PLASMA_TARGETS STREQUAL "Client")
    add_definitions(-DCLIENT)
endif(PLASMA_TARGETS STREQUAL "Client")

if(PLASMA_TARGETS STREQUAL "Patcher")
    add_definitions(-DPATCHER)
endif(PLASMA_TARGETS STREQUAL "Patcher")

if(PLASMA_TARGETS STREQUAL "Server")
    add_definitions(-DSERVER)
endif(PLASMA_TARGETS STREQUAL "Server")

if(PLASMA_TARGETS STREQUAL "NoAvMsgs")
    add_definitions(-DNO_AV_MSGS)
endif(PLASMA_TARGETS STREQUAL "NoAvMsgs")

if(PLASMA_TARGETS STREQUAL "Ethereal")
    add_definitions(-DSTREAM_LOGGER)
endif(PLASMA_TARGETS STREQUAL "Ethereal")

option(USE_VLD "Build and link with Visual Leak Detector (MSVC only)" OFF)
if(USE_VLD)
    add_definitions(-DUSE_VLD)
    find_package(VLD REQUIRED)
    include_directories(${VLD_INCLUDE_DIR})
endif(USE_VLD)

include(TestBigEndian)
test_big_endian(BIG_ENDIAN)
if(BIG_ENDIAN)
    add_definitions(-DBIG_ENDIAN)
else()
    add_definitions(-DLITTLE_ENDIAN)
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif(MSVC)

#TODO: Make the OpenSSL includes less promiscuous so this isn't needed
include_directories(${OPENSSL_INCLUDE_DIR})

add_subdirectory(Sources/Plasma)
add_subdirectory(Sources/Tools)
