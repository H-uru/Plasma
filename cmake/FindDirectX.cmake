if(WIN32)
    set(DirectX_LIBRARIES
        d3d9
        d3dcompiler
    )

    set(DirectX_FOUND TRUE)
else()
    set(DirectX_FOUND FALSE)
endif()
