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

set(DirectX_LIBRARIES
    ${DirectX_d3d9}
    ${DirectX_d3dx9}
)

if(DirectX_INCLUDE_DIR AND DirectX_d3d9 AND DirectX_d3dx9)
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
