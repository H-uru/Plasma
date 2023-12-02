include(FindPackageHandleStandardArgs)

if(WIN32)
    # 64-bit was introduced in 3ds Max 2008 (version 10) and 32-bit was dropped after 2013 (version 15)
    # Of course, 32-bit 2013 is basically impossible to find. Oh well.
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_3dsm_PATH_HINTS
            ENV ADSK_3DSMAX_SDK_2024
            ENV ADSK_3DSMAX_SDK_2023
            ENV ADSK_3DSMAX_SDK_2022
            ENV ADSK_3DSMAX_SDK_2021
            ENV ADSK_3DSMAX_SDK_2020
            ENV ADSK_3DSMAX_SDK_2019
            ENV ADSK_3DSMAX_SDK_2018
            ENV ADSK_3DSMAX_SDK_2017
            ENV ADSK_3DSMAX_SDK_2016
            ENV ADSK_3DSMAX_SDK_2015
            ENV ADSK_3DSMAX_SDK_2014
            ENV ADSK_3DSMAX_SDK_2013
            ENV ADSK_3DSMAX_SDK_2012
            ENV ADSK_3DSMAX_SDK_2011
            ENV ADSK_3DSMAX_SDK_2010
            ENV ADSK_3DSMAX_SDK_2009
            ENV ADSK_3DSMAX_SDK_2008
        )
    else()
        set(_3dsm_PATH_HINTS
            ENV ADSK_3DSMAX_SDK_2013
            ENV ADSK_3DSMAX_SDK_2012
            ENV ADSK_3DSMAX_SDK_2011
            ENV ADSK_3DSMAX_SDK_2010
            ENV ADSK_3DSMAX_SDK_2009
            ENV ADSK_3DSMAX_SDK_2008
            "C:/3dsmax7"
        )
    endif()

    find_path(3dsm_PATH NAMES "include/maxversion.h"
        PATHS ${_3dsm_PATH_HINTS}
        PATH_SUFFIXES maxsdk
    )

    find_path(3dsm_INCLUDE_DIR maxversion.h PATHS "${3dsm_PATH}/include")

    function(_filter_version RESULT_VARIABLE KEY)
        set(_FILTERED_VERSIONS ${_3dsm_VERSIONS})
        list(FILTER _FILTERED_VERSIONS INCLUDE
            REGEX "^[ \t]*#define[ \t]+${KEY}[ \t]+([0-9]+).*"
        )
        # Older maxen have multiple versions in the header. Sigh.
        list(POP_BACK _FILTERED_VERSIONS _VERSION_LINE)
        string(REGEX REPLACE "^.*${KEY}[ \t]+([0-9]+).*$" "\\1"
            _RESULT "${_VERSION_LINE}"
        )
        set(${RESULT_VARIABLE} "${_RESULT}" PARENT_SCOPE)
    endfunction()

    if(EXISTS "${3dsm_INCLUDE_DIR}/maxversion.h")
        file(STRINGS "${3dsm_INCLUDE_DIR}/maxversion.h" _3dsm_VERSIONS
            REGEX "^[ \t]*#define[ \t]+(MAX_VERSION_MAJOR|MAX_PRODUCT_YEAR_NUMBER)[ \t]+([0-9]+).*"
        )
        _filter_version(3dsm_VERSION "MAX_VERSION_MAJOR")
        _filter_version(3dsm_DISPLAY_VERSION "MAX_PRODUCT_YEAR_NUMBER")
        if(NOT 3dsm_DISPLAY_VERSION)
            set(3dsm_DISPLAY_VERSION "${3dsm_VERSION}")
        endif()
    endif()

    if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND 3dsm_VERSION VERSION_GREATER_EQUAL 10)
        set(_LIBPATH
            "${3dsm_PATH}/lib/x64/Release"
            "${3dsm_PATH}/x64/lib"
        )
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4 AND 3dsm_VERSION VERSION_LESS_EQUAL 15)
        set(_LIBPATH "${3dsm_PATH}/lib")
    elseif(DEFINED 3dsm_VERSION)
        message(WARNING "3ds Max ${3dsm_VERSION} does not support the target architecture")
    endif()

    if(DEFINED _LIBPATH)
        find_library(3dsm_BMM_LIBRARY bmm PATHS ${_LIBPATH})
        find_library(3dsm_CORE_LIBRARY core PATHS ${_LIBPATH})
        find_library(3dsm_CUSTDLG_LIBRARY CustDlg PATHS ${_LIBPATH})
        find_library(3dsm_GEOM_LIBRARY geom PATHS ${_LIBPATH})
        find_library(3dsm_GFX_LIBRARY gfx PATHS ${_LIBPATH})
        find_library(3dsm_GUP_LIBRARY gup PATHS ${_LIBPATH})
        find_library(3dsm_MANIPSYS_LIBRARY manipsys PATHS ${_LIBPATH})
        find_library(3dsm_MAXSCRPT_LIBRARY Maxscrpt PATHS ${_LIBPATH})
        find_library(3dsm_MAXUTIL_LIBRARY maxutil PATHS ${_LIBPATH})
        find_library(3dsm_MESH_LIBRARY mesh PATHS ${_LIBPATH})
        find_library(3dsm_MENUS_LIBRARY menus PATHS ${_LIBPATH})
        find_library(3dsm_MNMATH_LIBRARY mnmath PATHS ${_LIBPATH})
        find_library(3dsm_PARAMBLK2_LIBRARY paramblk2 PATHS ${_LIBPATH})
    endif()
endif()

find_package_handle_standard_args(
    3dsm
    REQUIRED_VARS 3dsm_BMM_LIBRARY
                  3dsm_CORE_LIBRARY
                  3dsm_CUSTDLG_LIBRARY
                  3dsm_GEOM_LIBRARY
                  3dsm_GFX_LIBRARY
                  3dsm_GUP_LIBRARY
                  3dsm_MANIPSYS_LIBRARY
                  3dsm_MAXSCRPT_LIBRARY
                  3dsm_MAXUTIL_LIBRARY
                  3dsm_MESH_LIBRARY
                  3dsm_MENUS_LIBRARY
                  3dsm_MNMATH_LIBRARY
                  3dsm_PARAMBLK2_LIBRARY
    VERSION_VAR 3dsm_VERSION
)

if(3dsm_FOUND AND NOT TARGET 3dsm)
    add_library(3dsm INTERFACE)
    set_property(
        TARGET 3dsm PROPERTY INTERFACE_LINK_LIBRARIES
            ${3dsm_BMM_LIBRARY}
            ${3dsm_CORE_LIBRARY}
            ${3dsm_CUSTDLG_LIBRARY}
            ${3dsm_GEOM_LIBRARY}
            ${3dsm_GFX_LIBRARY}
            ${3dsm_GUP_LIBRARY}
            ${3dsm_MANIPSYS_LIBRARY}
            ${3dsm_MAXSCRPT_LIBRARY}
            ${3dsm_MAXUTIL_LIBRARY}
            ${3dsm_MESH_LIBRARY}
            ${3dsm_MENUS_LIBRARY}
            ${3dsm_MNMATH_LIBRARY}
            ${3dsm_PARAMBLK2_LIBRARY}
    )
    set_target_properties(3dsm PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${3dsm_INCLUDE_DIR})
    if(3dsm_VERSION VERSION_GREATER_EQUAL 15)
        set_property(TARGET 3dsm PROPERTY INTERFACE_COMPILE_DEFINITIONS UNICODE _UNICODE)
    endif()
endif()

mark_as_advanced(
    3dsm_BMM_LIBRARY
    3dsm_CORE_LIBRARY
    3dsm_CUSTDLG_LIBRARY
    3dsm_GEOM_LIBRARY
    3dsm_GFX_LIBRARY
    3dsm_GUP_LIBRARY
    3dsm_MANIPSYS_LIBRARY
    3dsm_MAXSCRPT_LIBRARY
    3dsm_MAXUTIL_LIBRARY
    3dsm_MESH_LIBRARY
    3dsm_MENUS_LIBRARY
    3dsm_MNMATH_LIBRARY
    3dsm_PARAMBLK2_LIBRARY
)
