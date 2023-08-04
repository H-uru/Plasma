set(VCPKG_TARGET_ARCHITECTURE x64 arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_DISABLE_COMPILER_TRACKING TRUE)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES x86_64 arm64)
set(VCPKG_OSX_DEPLOYMENT_TARGET "10.14")

# This is a terrible hack because meson seems to suck.
if(PORT STREQUAL cairo)
    set(VCPKG_LIBRARY_LINKAGE dynamic)
    set(VCPKG_BUILD_TYPE release)
endif()

