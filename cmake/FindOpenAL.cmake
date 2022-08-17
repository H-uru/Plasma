# First, try to load OpenAL-Soft from its Cmake config.
find_package(OpenAL CONFIG QUIET)

# If that didn't work, fall back to the CMake find module
if(NOT TARGET OpenAL::OpenAL)
    unset(CMAKE_MODULE_PATH)
    find_package(OpenAL MODULE)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

    if(OPENAL_FOUND)
        if(NOT TARGET OpenAL::OpenAL)
            if(OPENAL_LIBRARY MATCHES "/([^/]+)\\.framework$")
                add_library(OpenAL::OpenAL INTERFACE IMPORTED)
                set_target_properties(
                    OpenAL::OpenAL PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES ${OPENAL_INCLUDE_DIR}
                    INTERFACE_LINK_LIBRARIES ${OPENAL_LIBRARY}
                )
            else()
                add_library(OpenAL::OpenAL UNKNOWN IMPORTED)
                set_target_properties(
                    OpenAL::OpenAL PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES ${OPENAL_INCLUDE_DIR}
                    IMPORTED_LOCATION ${OPENAL_LIBRARY}
                )
            endif()
        endif()
    elseif(OpenAL_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find OpenAL")
    endif()
endif()

# Vcpkg helpfully maps RelWithDebInfo and MinSizeRel for us. If we're not using the toolchain,
# though, we need to do that manually. But only if a Release configuration of OpenAL is available,
# otherwise we may simply break the whole target.
if(TARGET OpenAL::OpenAL)
    get_target_property(_IMPORTED_CONFIGS OpenAL::OpenAL IMPORTED_CONFIGURATIONS)
    if("RELEASE" IN_LIST _IMPORTED_CONFIGS)
        set_target_properties(
            OpenAL::OpenAL PROPERTIES
            MAP_IMPORTED_CONFIG_MINSIZEREL Release
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
        )
    endif()

    get_target_property(_INCLUDE_DIRS OpenAL::OpenAL INTERFACE_INCLUDE_DIRECTORIES)
    find_file(_EFX_H NAMES efx.h PATHS ${_INCLUDE_DIRS} NO_CACHE NO_DEFAULT_PATH)
    if(_EFX_H)
        set(OPENAL_HAS_EFX TRUE CACHE BOOL "Whether OpenAL includes EFX support")
    endif()
endif()
