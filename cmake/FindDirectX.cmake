option(DirectX_OLD_SDK "Is this an old (November 2008) version of the SDK?" OFF)

if (DirectX_OLD_SDK)
    add_definitions(-DDX_OLD_SDK)
endif(DirectX_OLD_SDK)


if(DirectX_INCLUDE_DIR)
    set(DirectX_FIND_QUIETLY TRUE)
endif()

# Figure out the arch for the path suffixes
if(CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(_dxarch "x64")
else()
    set(_dxarch "x86")
endif()


find_path(DirectX_INCLUDE_DIR d3dx9.h
          PATHS "$ENV{DXSDK_DIR}/Include"
)

find_library(DirectX_d3d9 NAMES d3d9
             PATHS "$ENV{DXSDK_DIR}/Lib/${_dxarch}"
)

find_library(DirectX_d3dx9 NAMES d3dx9
             PATHS "$ENV{DXSDK_DIR}/Lib/${_dxarch}"
)

find_library(DirectX_dinput8 NAMES dinput8
             PATHS "$ENV{DXSDK_DIR}/Lib/${_dxarch}"
)

find_library(DirectX_dsound NAMES dsound
             PATHS "$ENV{DXSDK_DIR}/Lib/${_dxarch}"
)

find_library(DirectX_dxguid NAMES dxguid
             PATHS "$ENV{DXSDK_DIR}/Lib/${_dxarch}"
)

if (DirectX_OLD_SDK)
    find_library(DirectX_dxerr NAMES dxerr9
                 PATHS "$ENV{DXSDK_DIR}/Lib/${_dxarch}"
    )
else()
    find_library(DirectX_dxerr NAMES DxErr
                 PATHS "$ENV{DXSDK_DIR}/Lib/${_dxarch}"
    )
endif(DirectX_OLD_SDK)

set(DirectX_LIBRARIES
    ${DirectX_d3d9}
    ${DirectX_d3dx9}
    ${DirectX_dinput8}
    ${DirectX_dsound}
    ${DirectX_dxguid}
    ${DirectX_dxerr}
)


if(DirectX_INCLUDE_DIR AND DirectX_d3d9 AND DirectX_d3dx9 AND DirectX_dinput8
                       AND DirectX_dsound AND DirectX_dxguid AND DirectX_dxerr)
    set(DirectX_FOUND TRUE)
endif()

if (DirectX_FOUND)
    if(NOT DirectX_FIND_QUIETLY)
        message(STATUS "Found DirectX SDK: ${DirectX_INCLUDE_DIR}")
    endif()
else()
    if(DirectX_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find DirectX SDK")
    endif()
endif()
