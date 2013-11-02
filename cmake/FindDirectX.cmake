option(DirectX_OLD_SDK "Is this an old (November 2008) version of the SDK?" OFF)

if(NOT DirectX_FOUND)
    FIND_PATH(DirectX_INCLUDE_DIR d3dx9.h
        "$ENV{DXSDK_DIR}/Include"
        "C:/Program Files/Microsoft Visual Studio .NET 2003/Vc7/PlatformSDK/Include"
        "C:/Program Files/Microsoft DirectX SDK (February 2006)/Include"
        "C:/Program Files/Microsoft DirectX 9.0 SDK (June 2005)/Include"
        "C:/DXSDK/Include"
        DOC "path of directx includes files"
    )

    # Figure out the arch for the path suffixes
    if(CMAKE_SIZEOF_VOID_P MATCHES "8")
        set(_dxarch "x64")
    else()
        set(_dxarch "x86")
    endif()

    get_filename_component(_dxpath ${DirectX_INCLUDE_DIR} PATH)

    find_library(DirectX_d3d9 NAMES d3d9
                 PATHS "${_dxpath}/Lib/${_dxarch}" "${_dxpath}/Lib"
    )

    find_library(DirectX_d3dx9 NAMES d3dx9
                 PATHS "${_dxpath}/Lib/${_dxarch}" "${_dxpath}/Lib"
    )

    find_library(DirectX_dinput8 NAMES dinput8
                 PATHS "${_dxpath}/Lib/${_dxarch}" "${_dxpath}/Lib"
    )

    find_library(DirectX_dsound NAMES dsound
                 PATHS "${_dxpath}/Lib/${_dxarch}" "${_dxpath}/Lib"
    )

    find_library(DirectX_dxguid NAMES dxguid
                 PATHS "${_dxpath}/Lib/${_dxarch}" "${_dxpath}/Lib"
    )

    find_library(DirectX_dxerr NAMES dxerr dxerr9
                 PATHS "${_dxpath}/Lib/${_dxarch}" "${_dxpath}/Lib"
    )


    if(DirectX_INCLUDE_DIR)
        if (DirectX_d3d9 AND DirectX_d3dx9 AND DirectX_dinput8
                         AND DirectX_dsound AND DirectX_dxguid AND DirectX_dxerr)
            set(DirectX_FOUND TRUE CACHE BOOL "" FORCE)
            mark_as_advanced(DirectX_FOUND)
            message(STATUS "Found DirectX SDK: ${_dxpath}")
        elseif(DirectX_FIND_REQUIRED)
            message(FATAL_ERROR "Could not find DirectX SDK libraries")
        endif()
    elseif(DirectX_FIND_REQUIRED)
        message(FATAL_ERROR
                "DirectX SDK not found, install it or set DirectX_INCLUDE_DIR manually."
        )
    endif()
    
    if (DirectX_dxerr MATCHES ".*/[dD][xX][eE][rR][rR]9.*")
        set(DirectX_OLD_SDK ON CACHE BOOL "Is this an old (November 2008) version of the SDK?" FORCE)
        message(STATUS "old Directx SDK detected")
    endif()
endif(NOT DirectX_FOUND)

if (${DirectX_OLD_SDK})
    add_definitions(-DDX_OLD_SDK)
endif()

# NOTE: In some cases, includes of Windows SDK must be used BEFORE includes of DirectX SDK.
#       Otherwise, it can make error about PVOID64!
get_filename_component(WindowsSdkDir
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows;CurrentInstallFolder]"
    ABSOLUTE CACHE
)
set(DirectX_INCLUDE_DIR "${WindowsSdkDir}/include" "${DirectX_INCLUDE_DIR}")

set(DirectX_LIBRARIES
    ${DirectX_d3d9}
    ${DirectX_d3dx9}
    ${DirectX_dinput8}
    ${DirectX_dsound}
    ${DirectX_dxguid}
    ${DirectX_dxerr}
)
