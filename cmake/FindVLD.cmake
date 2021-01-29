include(FindPackageHandleStandardArgs)

if(MSVC)
    # The parens in this env variable give CMake heartburn, so we whisper sweet nothings.
    set(_PROGRAMFILES "PROGRAMFILES(X86)")
    set(_PROGRAMFILES "$ENV{${_PROGRAMFILES}}")

    find_path(
        VLD_INCLUDE_DIR
        NAMES vld.h
        PATHS "${_PROGRAMFILES}/Visual Leak Detector/include"
    )

    if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
        set(_VLD_LIB "Win32")
    elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        set(_VLD_LIB "Win64")
    endif()

    find_library(
        VLD_LIBRARY
        NAMES vld
        PATHS "${_PROGRAMFILES}/Visual Leak Detector/lib/${_VLD_LIB}"
    )
endif()

find_package_handle_standard_args(VLD REQUIRED_VARS VLD_INCLUDE_DIR VLD_LIBRARY)

set(VLD_INCLUDE_DIRS ${VLD_INCLUDE_DIR})
set(VLD_LIBRARIES ${VLD_LIBRARY})
