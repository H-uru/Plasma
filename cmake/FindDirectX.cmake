if(WIN32)
    set(DirectX_LIBRARIES
        d3d9
        D3Dcompiler
    )

    set(DirectX_FOUND TRUE)
else()
    set(DirectX_FOUND FALSE)
endif()
