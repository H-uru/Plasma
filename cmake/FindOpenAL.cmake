# Derived from the builtin FindOpenAL.cmake, but with less idiocy.

include(FindPackageHandleStandardArgs)

find_path(
    OpenAL_INCLUDE_DIR al.h
    HINTS ENV OpenALDIR
    PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /opt
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Creative\ Labs\\OpenAL\ 1.1\ Software\ Development\ Kit\\1.00.0000;InstallDir]
        PATH_SUFFIXES include/AL include/OpenAL include AL OpenAL
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_OpenAL_ARCH_DIR libs/Win64)
else()
    set(_OpenAL_ARCH_DIR libs/Win32)
endif()

find_library(
    OpenAL_LIBRARY
    NAMES OpenAL al openal OpenAL32
    HINTS ENV OpenALDIR
    PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /opt
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Creative\ Labs\\OpenAL\ 1.1\ Software\ Development\ Kit\\1.00.0000;InstallDir]
        PATH_SUFFIXES libx32 lib64 lib libs64 libs ${_OpenAL_ARCH_DIR}
)

unset(_OpenAL_ARCH_DIR)

find_package_handle_standard_args(
    OpenAL
    REQUIRED_VARS OpenAL_LIBRARY OpenAL_INCLUDE_DIR
    VERSION_VAR OpenAL_VERSION_STRING
)

mark_as_advanced(OpenAL_LIBRARY OpenAL_INCLUDE_DIR)

if(NOT TARGET OpenAL::OpenAL)
    add_library(OpenAL::OpenAL UNKNOWN IMPORTED)
    set_target_properties(
        OpenAL::OpenAL PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${OpenAL_INCLUDE_DIR}
        IMPORTED_LOCATION ${OpenAL_LIBRARY}
    )
endif()

set(OpenAL_INCLUDE_DIRS ${OpenAL_INCLUDE_DIR})
set(OpenAL_LIBRARIES ${OpenAL_LIBRARY})
