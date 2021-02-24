include(FindPackageHandleStandardArgs)

if(WIN32)
    # Karnage: Be sure to select a 64-bit capable 3ds Max SDK on 64-bit builds.
    set(_TEST_FILES "include/maxapi.h")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_LIBDIR "x64/lib")
        list(APPEND _TEST_FILES "x64/lib/gup.lib")
    else()
        set(_LIBDIR "lib")
    endif()

    find_path(3dsm_PATH NAMES ${_TEST_FILES}
        PATHS "C:/3dsmax7" ENV ADSK_3DSMAX_SDK_2012
        PATH_SUFFIXES maxsdk
    )

    find_path(3dsm_INCLUDE_DIR maxapi.h PATHS "${3dsm_PATH}/include")

    set(_LIBPATH "${3dsm_PATH}/${_LIBDIR}")
    find_library(3dsm_BMM_LIBRARY bmm PATHS "${_LIBPATH}")
    find_library(3dsm_CORE_LIBRARY core PATHS "${_LIBPATH}")
    find_library(3dsm_CUSTDLG_LIBRARY CustDlg PATHS "${_LIBPATH}")
    find_library(3dsm_GEOM_LIBRARY geom PATHS "${_LIBPATH}")
    find_library(3dsm_GFX_LIBRARY gfx PATHS "${_LIBPATH}")
    find_library(3dsm_GUP_LIBRARY gup PATHS "${_LIBPATH}")
    find_library(3dsm_MANIPSYS_LIBRARY manipsys PATHS "${_LIBPATH}")
    find_library(3dsm_MAXSCRPT_LIBRARY Maxscrpt PATHS "${_LIBPATH}")
    find_library(3dsm_MAXUTIL_LIBRARY maxutil PATHS "${_LIBPATH}")
    find_library(3dsm_MESH_LIBRARY mesh PATHS "${_LIBPATH}")
    find_library(3dsm_MENUS_LIBRARY menus PATHS "${_LIBPATH}")
    find_library(3dsm_MNMATH_LIBRARY mnmath PATHS "${_LIBPATH}")
    find_library(3dsm_PARAMBLK2_LIBRARY paramblk2 PATHS "${_LIBPATH}")
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
