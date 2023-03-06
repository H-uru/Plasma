include(FindPackageHandleStandardArgs)
find_package(DirectXMath CONFIG QUIET)

if(TARGET Microsoft::DirectXMath)
    find_package_handle_standard_args(DirectXMath CONFIG_MODE)
    target_compile_definitions(Microsoft::DirectXMath
        INTERFACE
            PAL_STDCPP_COMPAT
            _XM_NO_XMVECTOR_OVERLOADS_
    )
elseif(WIN32)
    # DirectXMath should be part of the Windows SDK, but verify that.
    include(CheckIncludeFileCXX)
    CHECK_INCLUDE_FILE_CXX("DirectXMath.h" DirectXMath_FOUND)

    # Add a dummy target to match what the upstream DirectXMath does. There is no need to
    # add anything to this target because DirectXMath.h is available in the Windows SDK, which
    # is always available.
    if(DirectXMath_FOUND)
        add_library(Microsoft::DirectXMath UNKNOWN IMPORTED)
    endif()
endif()
