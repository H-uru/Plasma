project(Plasma)
cmake_minimum_required(VERSION 2.8)

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

if(WIN32)
    find_package(PhysX REQUIRED)  #TODO: Not required if we aren't building the client
    find_package(DirectX REQUIRED)
    find_package(Freetype)
endif(WIN32)

find_package(MaxSDK)          #TODO: Only find this if we are building PlasmaMax
find_package(Bink)            #TODO: Find Bink, but don't require it if plPipeline isn't built...
#                                    Or better yet, just eliminate bink altogether

# libCurl isn't smart enough to detect this for us, so we have to configure it ourselves
option(CURL_IS_STATIC "Using the static version of libcurl?" ON)
if(CURL_IS_STATIC)
    add_definitions(-DCURL_STATICLIB)
endif(CURL_IS_STATIC)

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

# TODO: Maybe some kind of automated test here?
option(PLASMA_USE_SSE "Enable SSE optimizations?" ON)
if(PLASMA_USE_SSE)
    add_definitions(-DHAVE_SSE)
endif(PLASMA_USE_SSE)

#TODO: Make the OpenSSL includes less promiscuous so this isn't needed
include_directories(${OPENSSL_INCLUDE_DIR})

add_subdirectory(Sources/Plasma)
add_subdirectory(Sources/Tools)
