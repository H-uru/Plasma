project(Plasma)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Find all 3rd-party libraries that are required
find_package(OpenSSL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(EXPAT REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PHYSX REQUIRED)  #TODO: Not required if we aren't building the client
find_package(Ogg REQUIRED)    #TODO: Not required if we aren't building the client
find_package(Vorbis REQUIRED) #TODO: Not required if we aren't building the client
find_package(Speex REQUIRED)  #TODO: Not required if we aren't building the client
find_package(DirectX REQUIRED)
find_package(Bink)            #TODO: Find Bink, but don't require it if plPipeline isn't built...
#                                    Or better yet, just eliminate bink altogether

option(PLASMA_EXTERNAL_RELEASE "Is this release intended for the general public?" OFF)

if(PLASMA_EXTERNAL_RELEASE)
	add_definitions(-DPLASMA_EXTERNAL_RELEASE)
endif(PLASMA_EXTERNAL_RELEASE)

set(PLASMA_BUILD_TYPE "Live"
	CACHE STRING "Which type of client to build")
set_property(CACHE PLASMA_BUILD_TYPE PROPERTY STRINGS
             "Dev" "QA" "Test" "Beta" "Live")

if(PLASMA_BUILD_TYPE STREQUAL "Dev")
	add_definitions(-DBUILD_TYPE=BUILD_TYPE_DEV)
elseif(PLASMA_BUILD_TYPE STREQUAL "QA")
	add_definitions(-DBUILD_TYPE=BUILD_TYPE_QA)
elseif(PLASMA_BUILD_TYPE STREQUAL "Test")
	add_definitions(-DBUILD_TYPE=BUILD_TYPE_TEST)
elseif(PLASMA_BUILD_TYPE STREQUAL "Beta")
	add_definitions(-DBUILD_TYPE=BUILD_TYPE_BETA)
elseif(PLASMA_BUILD_TYPE STREQUAL "Live")
	add_definitions(-DBUILD_TYPE=BUILD_TYPE_LIVE)
endif(PLASMA_BUILD_TYPE STREQUAL "Dev")

set(PLASMA_TARGETS "Client"
    CACHE STRING "Which set of plasma targets to build and use")
set_property(CACHE PLASMA_TARGETS PROPERTY STRINGS
             "Client" "Server" "Patcher" "Ethereal" "NoAvMsgs")

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

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif(MSVC)

add_subdirectory(Sources/Plasma)
# add_subdirectory(Sources/Tools)
