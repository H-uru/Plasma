project(CyanWorldsEngine)
cmake_minimum_required(VERSION 2.8)

# Find all 3rd-party libraries that are required
find_package(OpenSSL REQUIRED)
find_package(OpenAL REQUIRED)

option(EXTERNAL_RELEASE "Is this release intended for the general public?" OFF)

# TODO: are there any other build types which are useful for us?
add_definitions(-DBUILD_TYPE_LIVE)

if(EXTERNAL_RELEASE)
	add_definitions(-DPLASMA_EXTERNAL_RELEASE)
endif(EXTERNAL_RELEASE)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	add_definitions(-D_DEBUG)
endif(CMAKE_BUILD_TYPE STREQUAL Debug)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

add_subdirectory(Sources/Plasma)
# add_subdirectory(Sources/Tools)
