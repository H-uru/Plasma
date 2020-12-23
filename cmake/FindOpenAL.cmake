# First, try to load OpenAL-Soft from its Cmake config.
find_package(OpenAL CONFIG QUIET)

# If that didn't work, fall back to the CMake find module
if(NOT TARGET OpenAL::OpenAL)
    unset(CMAKE_MODULE_PATH)
    find_package(OpenAL MODULE)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

    if(OPENAL_FOUND)
        if(NOT TARGET OpenAL::OpenAL)
            add_library(OpenAL::OpenAL UNKNOWN IMPORTED)
            set_target_properties(
                OpenAL::OpenAL PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${OPENAL_INCLUDE_DIR}
                IMPORTED_LOCATION ${OPENAL_LIBRARY}
            )
        endif()
    elseif(OpenAL_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find OpenAL")
    endif()
endif()

# Upstream neglects to do this -- vcpkg masks it for us, but you are in trouble if you are
# not using the vcpkg toolchain.
if(TARGET OpenAL::OpenAL)
    set_target_properties(
        OpenAL::OpenAL PROPERTIES
        MAP_IMPORTED_CONFIG_MINSIZEREL Release
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    )
endif()
