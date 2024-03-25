set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_DISABLE_COMPILER_TRACKING TRUE)

# Unfortunately, we cannot include() anything from here because CMAKE_CURRENT_LIST_DIR is "wrong."
# If you update this list, remember to synchronize x86-windows-plasma.cmake.
set(_PLASMA_DYNAMIC_LIBRARIES
    cairo
    python2
    python3
)

cmake_policy(SET CMP0057 NEW)
if(PORT IN_LIST _PLASMA_DYNAMIC_LIBRARIES)
    set(VCPKG_LIBRARY_LINKAGE dynamic)
else()
    set(VCPKG_LIBRARY_LINKAGE static)
endif()

# This is a terrible hack because meson seems to suck.
if(PORT STREQUAL cairo)
    set(VCPKG_BUILD_TYPE release)
endif()
