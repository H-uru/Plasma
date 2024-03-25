if(VCPKG_TARGET_IS_WINDOWS)
    vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)
endif()

if(VCPKG_TARGET_IS_LINUX AND NOT EXISTS "/usr/share/doc/libgles2/copyright")
    message(STATUS "libgles2-mesa-dev must be installed before libepoxy can build. Install it with \"apt-get install libgles2-mesa-dev\".")
endif()

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO anholt/libepoxy
    REF 1.5.10
    SHA512 6786f31c6e2865e68a90eb912900a86bf56fd3df4d78a477356886ac3b6ef52ac887b9c7a77aa027525f868ae9e88b12e5927ba56069c2e115acd631fca3abee
    HEAD_REF master
    PATCHES
        threading.patch
)

# TODO: Enable EGL on Windows
if (VCPKG_TARGET_IS_WINDOWS OR VCPKG_TARGET_IS_OSX)
    set(OPTIONS -Dglx=no -Degl=no -Dx11=false)
else()
    set(OPTIONS -Dglx=yes -Degl=yes -Dx11=true)
endif()
if(VCPKG_TARGET_IS_WINDOWS)
    list(APPEND OPTIONS -Dc_std=c99)
endif()

vcpkg_configure_meson(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${OPTIONS}
        -Dtests=false
)
vcpkg_install_meson()
vcpkg_copy_pdbs()

vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/pkgconfig")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share/pkgconfig")

file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
