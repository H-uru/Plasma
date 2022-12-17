include(SubmoduleUtils)
plasma_init_git_submodule("Sources/Tests/gtest" RESULT has_gtest_module)
if(NOT has_gtest_module)
    return()
endif()

# Why is this OFF by default?  It doesn't work with CMake's defaults, and
# should really only be off if the project has specifically been set up to
# use the static MSVC CRT
set(gtest_force_shared_crt ON CACHE INTERNAL "Override gtest default")

# Force other gtest settings to line up correctly -- we're compiling
# Plasma, not gtest...
set(gtest_build_samples OFF CACHE INTERNAL "Override gtest default")
set(gtest_build_tests OFF CACHE INTERNAL "Override gtest default")
set(gtest_disable_pthreads OFF CACHE INTERNAL "Override gtest default")
set(gtest_hide_internal_symbols ON CACHE INTERNAL "Override gtest default")

set(GOOGLETEST_VERSION 1.12.1)
set(INSTALL_GTEST OFF CACHE INTERNAL "Override gtest default" FORCE)
add_subdirectory(gtest EXCLUDE_FROM_ALL)

include_directories(${GTEST_INCLUDE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_subdirectory(CoreTests)
add_subdirectory(NucleusTests)
add_subdirectory(PubUtilTests)
add_subdirectory(FeatureTests)
