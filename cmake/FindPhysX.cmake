include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

if(CMAKE_SIZEOF_VOID_P EQUAL "8")
    set(_PHYSX_LIBRARY_SUFFIX "64")
else()
    set(_PHYSX_LIBRARY_SUFFIX "32")
endif()

set(_PHYSX_COMMON_LIBRARY_NAMES
    PhysXCommon
    PhysXCommon_static
    PhysXCommon_${_PHYSX_LIBRARY_SUFFIX}
    PhysXCommon_static_${_PHYSX_LIBRARY_SUFFIX}
    libPhysXCommon
    libPhysXCommon_static
    libPhysXCommon_${_PHYSX_LIBRARY_SUFFIX}
    libPhysXCommon_static_${_PHYSX_LIBRARY_SUFFIX}
)
set(_PHYSX_PHYSICS_LIBRARY_NAMES
    PhysX
    PhysX_static
    PhysX_${_PHYSX_LIBRARY_SUFFIX}
    PhysX_static_${_PHYSX_LIBRARY_SUFFIX}
    libPhysX
    libPhysX_static
    libPhysX_${_PHYSX_LIBRARY_SUFFIX}
    libPhysX_static_${_PHYSX_LIBRARY_SUFFIX}
)
set(_PHYSX_FOUNDATION_LIBRARY_NAMES
    PhysXFoundation
    PhysXFoundation_static
    PhysXFoundation_${_PHYSX_LIBRARY_SUFFIX}
    PhysXFoundation_static_${_PHYSX_LIBRARY_SUFFIX}
    libPhysXFoundation
    libPhysXFoundation_static
    libPhysXFoundation_${_PHYSX_LIBRARY_SUFFIX}
    libPhysXFoundation_static_${_PHYSX_LIBRARY_SUFFIX}
)
set(_PHYSX_COOKING_LIBRARY_NAMES
    PhysXCooking
    PhysXCooking_static
    PhysXCooking_${_PHYSX_LIBRARY_SUFFIX}
    PhysXCooking_static_${_PHYSX_LIBRARY_SUFFIX}
    libPhysXCooking
    libPhysXCooking_static
    libPhysXCooking_${_PHYSX_LIBRARY_SUFFIX}
    libPhysXCooking_static_${_PHYSX_LIBRARY_SUFFIX}
)
set(_PHYSX_EXTENSIONS_LIBRARY_NAMES
    PhysXExtensions
    PhysXExtensions_static
    PhysXExtensions_${_PHYSX_LIBRARY_SUFFIX}
    PhysXExtensions_static_${_PHYSX_LIBRARY_SUFFIX}
    libPhysXExtensions
    libPhysXExtensions_static
    libPhysXExtensions_${_PHYSX_LIBRARY_SUFFIX}
    libPhysXExtensions_static_${_PHYSX_LIBRARY_SUFFIX}
)
set(_PHYSX_CHARACTER_LIBRARY_NAMES
    PhysXCharacterKinematic
    PhysXCharacterKinematic_static
    PhysXCharacterKinematic_${_PHYSX_LIBRARY_SUFFIX}
    PhysXCharacterKinematic_static_${_PHYSX_LIBRARY_SUFFIX}
    libPhysXCharacterKinematic
    libPhysXCharacterKinematic_static
    libPhysXCharacterKinematic_${_PHYSX_LIBRARY_SUFFIX}
    libPhysXCharacterKinematic_static_${_PHYSX_LIBRARY_SUFFIX}
)
set(_PHYSX_PVD_LIBRARY_NAMES
    PhysXPvdSDK
    PhysXPvdSDK_static
    PhysXPvdSDK_${_PHYSX_LIBRARY_SUFFIX}
    PhysXPvdSDK_static_${_PHYSX_LIBRARY_SUFFIX}
    libPhysXPvdSDK
    libPhysXPvdSDK_static
    libPhysXPvdSDK_${_PHYSX_LIBRARY_SUFFIX}
    libPhysXPvdSDK_static_${_PHYSX_LIBRARY_SUFFIX}
)

# Stoopid vcpkg build debug and optimized libraries with the same name but in different directories.
foreach(prefix_path IN LISTS CMAKE_PREFIX_PATH)
    if(${prefix_path} MATCHES "[Dd][Ee][Bb][Uu][Gg]\/?$")
        list(APPEND _PHYSX_DEBUG_PATHS ${prefix_path})
    else()
        list(APPEND _PHYSX_RELEASE_PATHS ${prefix_path})
    endif()
    list(APPEND _PHYSX_PREFIX ${prefix_path})
endforeach()

# For non-vcpkg builds... *Gulp*
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(MSVC)
        set(_PHYSX_BIN_DIR "win.")
        if(CMAKE_SIZEOF_VOID_P EQUAL "8")
            string(APPEND _PHYSX_BIN_DIR "x86_64.")
        else()
            string(APPEND _PHYSX_BIN_DIR "x86.")
        endif()
        string(APPEND _PHYSX_BIN_DIR "vc${MSVC_TOOLSET_VERSION}.mt")
    endif()

    if(MSVC_TOOLSET_VERSION EQUAL 120)
        set(_PHYSX_DIR "vc12win${_PHYSX_LIBRARY_SUFFIX}")
    # There is no toolset version 130...
    elseif(MSVC_TOOLSET_VERSION EQUAL 140)
        set(_PHYSX_DIR "vc14win${_PHYSX_LIBRARY_SUFFIX}")
    elseif(MSVC_TOOLSET_VERSION EQUAL 141)
        set(_PHYSX_DIR "vc15win${_PHYSX_LIBRARY_SUFFIX}")
    elseif(MSVC_TOOLSET_VERSION EQUAL 142)
        set(_PHYSX_DIR "vc16win${_PHYSX_LIBRARY_SUFFIX}")
    elseif(NOT VCPKG_TOOLCHAIN)
        message(WARNING "PhysX: Unhandled MSVC Toolset ${MSVC_TOOLSET_VERSION}. You may need to manually specify artifacts.")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(_PHYSX_DIR "linux")
    set(_PHYSX_BIN_DIR "linux.clang")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(_PHYSX_DIR "mac64")
    set(_PHYSX_BIN_DIR "mac64.xcode")
elseif(NOT VCPKG_TOOLCHAIN)
    message(WARNING "PhysX: Unhandled system: ${CMAKE_SYSTEM_NAME}. You may need to manually specify artifacts.")
endif()

if(_PHYSX_DIR)
    foreach(prefix_path ${CMAKE_PREFIX_PATH})
        if(_PHYSX_BIN_DIR)
            list(APPEND _PHYSX_DEBUG_PATHS "${prefix_path}/${_PHYSX_DIR}/PhysX/bin/${_PHYSX_BIN_DIR}/debug")
            list(APPEND _PHYSX_RELEASE_PATHS "${prefix_path}/${_PHYSX_DIR}/PhysX/bin/${_PHYSX_BIN_DIR}/release")
        elseif(NOT VCPKG_TOOLCHAIN)
            list(APPEND _PHYSX_DEBUG_PATHS ${prefix_path})
            list(APPEND _PHYSX_RELEASE_PATHS ${prefix_path})
        endif()
        list(APPEND _PHYSX_PREFIX "${prefix_path}/${_PHYSX_DIR}/PhysX")
        list(APPEND _PHYSX_PREFIX "${prefix_path}/${_PHYSX_DIR}/PxShared")
    endforeach()
elseif(NOT VCPKG_TOOLCHAIN)
    set(_PHYSX_DEBUG_PATHS ${CMAKE_PREFIX_PATH})
    set(_PHYSX_RELEASE_PATHS ${CMAKE_PREFIX_PATH})
endif()

find_path(PHYSX_INCLUDE_DIR NAMES PxPhysicsAPI.h
          PATHS ${_PHYSX_PREFIX}
          PATH_SUFFIXES include/physx include
          NO_CMAKE_PATH
)
mark_as_advanced(PHYSX_INCLUDE_DIR)

# PhysX has two include directories, but vcpkg merges them together. In case of a manual build,
# check and compensate for the lack of merging here.
if(PHYSX_INCLUDE_DIR)
    if(NOT EXISTS ${PHYSX_INCLUDE_DIR}/foundation/Px.h)
        find_path(PHYSX_FOUNDATION_INCLUDE_DIR NAMES foundation/Px.h
                  PATHS ${_PHYSX_PREFIX}
                  PATH_SUFFIXES include/physx include
                  NO_CMAKE_PATHS
        )
        mark_as_advanced(PHYSX_FOUNDATION_INCLUDE_DIR)
    else()
        set(PHYSX_FOUNDATION_INCLUDE_DIR ${PHYSX_INCLUDE_DIR} CACHE PATH "Path to a file.")
    endif()
endif()

macro(_find_physx_library SUFFIX)
    cmake_parse_arguments(_fpl "FOUNDATION_INCLUDE" "" "INTERFACE_LIBS" ${ARGN})

    string(TOUPPER ${SUFFIX} _SUFFIX_UPPER)
    set(VAR_NAME "PHYSX_${_SUFFIX_UPPER}")
    set(TARGET "PhysX::${SUFFIX}")

    find_library(${VAR_NAME}_LIBRARY_RELEASE
                 NAMES ${_${VAR_NAME}_LIBRARY_NAMES}
                 PATHS ${_PHYSX_RELEASE_PATHS}
                 PATH_SUFFIXES lib
                 NO_CMAKE_PATH
    )
    find_library(${VAR_NAME}_LIBRARY_DEBUG
                 NAMES ${_${VAR_NAME}_LIBRARY_NAMES}
                 PATHS ${_PHYSX_DEBUG_PATHS}
                 PATH_SUFFIXES lib
                 NO_CMAKE_PATH
    )
    select_library_configurations(${VAR_NAME})

    if(${VAR_NAME}_LIBRARY AND NOT TARGET ${TARGET})
        add_library(${TARGET} UNKNOWN IMPORTED)

        if(DEFINED _fpl_FOUNDATION_INCLUDE AND EXISTS "${PHYSX_FOUNDATION_INCLUDE_DIR}")
            set_property(
                TARGET ${TARGET} APPEND PROPERTY
                INTERFACE_INCLUDE_DIRECTORIES ${PHYSX_FOUNDATION_INCLUDE_DIR}
            )
        endif()

        if(EXISTS "${PHYSX_INCLUDE_DIR}")
            set_property(
                TARGET ${TARGET} APPEND PROPERTY
                INTERFACE_INCLUDE_DIRECTORIES ${PHYSX_INCLUDE_DIR}
            )
        else()
            message(FATAL_ERROR "PhysX include directory missing: ${PHYSX_INCLUDE_DIR}")
        endif()

        if(EXISTS "${${VAR_NAME}_LIBRARY_DEBUG}" AND EXISTS "${${VAR_NAME}_LIBRARY_RELEASE}")
            set_target_properties(
                ${TARGET} PROPERTIES
                IMPORTED_LOCATION_DEBUG ${${VAR_NAME}_LIBRARY_DEBUG}
                IMPORTED_LOCATION_RELEASE ${${VAR_NAME}_LIBRARY_RELEASE}
                MAP_IMPORTED_CONFIG_MINSIZEREL Release
                MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
            )
        elseif(EXISTS "${${VAR_NAME}_LIBRARY}")
            set_target_properties(
                ${TARGET} PROPERTIES
                IMPORTED_LOCATION ${${VAR_NAME}_LIBRARY}
            )
        else()
            message(FATAL_ERROR "PhysX ${SUFFIX} library missing: ${${VAR_NAME}_LIBRARY}")
        endif()

        if(DEFINED _fpl_INTERFACE_LIBS)
            set_property(
                TARGET ${TARGET} APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES ${_fpl_INTERFACE_LIBS}
            )
        endif()
    endif()

    unset(TARGET)
    unset(VAR_NAME)
    unset(_SUFFIX_UPPER)
endmacro()

_find_physx_library(Common)
_find_physx_library(Physics INTERFACE_LIBS ${CMAKE_DL_LIBS})
_find_physx_library(Foundation FOUNDATION_INCLUDE)
_find_physx_library(Cooking)
_find_physx_library(Extensions)
_find_physx_library(Character)
_find_physx_library(PVD)

find_package_handle_standard_args(PhysX
                                  REQUIRED_VARS PHYSX_COMMON_LIBRARY
                                                PHYSX_PHYSICS_LIBRARY
                                                PHYSX_FOUNDATION_LIBRARY
                                                PHYSX_COOKING_LIBRARY
                                                PHYSX_EXTENSIONS_LIBRARY
                                                PHYSX_CHARACTER_LIBRARY
                                                PHYSX_PVD_LIBRARY
                                                PHYSX_INCLUDE_DIR
                                                PHYSX_FOUNDATION_INCLUDE_DIR
                                  REASON_FAILURE_MESSAGE "Be sure that PhysX 4.1 is available."
)

if(PhysX_FOUND AND NOT TARGET PhysX::PhysX)
    add_library(PhysX::PhysX INTERFACE IMPORTED)
    set_property(
        TARGET PhysX::PhysX APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES PhysX::Character PhysX::Extensions PhysX::Physics
                                 PhysX::PVD PhysX::Cooking PhysX::Common PhysX::Foundation
    )
endif()
