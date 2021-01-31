# This port doesn't include an "include" directory or a "copyright" file, so it's considered
# empty by vcpkg. This will fail the validation unless we mark the port empty.
set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
if(NOT VCPKG_TARGET_IS_WINDOWS)
    message(STATUS "Install cairo on Ubuntu with `sudo apt install libcairo2` and on macOS with `brew install cairo`.")
endif()

set(CAIROSVG_VERSION "2.5.1")
set(CURRENT_PYTHON3_PREFIX "${CURRENT_PACKAGES_DIR}/tools/python3")
set(INSTALLED_PYTHON3_PREFIX "${CURRENT_INSTALLED_DIR}/tools/python3")

if(EXISTS CURRENT_PACKAGES_DIR)
    file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}")
endif()

# Copy over cairo.dll as libcairo-2.dll on Windows only. Other OSes should install cairo from
# the system package manager. This will be tested by the non-platform specific bits.
if(VCPKG_TARGET_IS_WINDOWS)
    # We are running in script mode, so no toolchains are available. Sad.
    list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES ${VCPKG_TARGET_SHARED_LIBRARY_SUFFIX})
    find_program(
        PYTHON3_EXECUTABLE
        NAMES python python3 python3.9
        PATHS "${INSTALLED_PYTHON3_PREFIX}"
        NO_DEFAULT_PATH
    )
    find_library(
        CAIRO_DLL_RELEASE
        NAMES cairo cairo-2
        PATHS "${CURRENT_INSTALLED_DIR}/bin"
    )

    if(NOT CAIRO_DLL_RELEASE)
        message(FATAL_ERROR "Unable to find cairo DLL. Be sure it was built as a shared library.")
    endif()

    # Python 3.8 tightened up the rules for DLL loading pretty significantly. One of the few ways to
    # make this work is to put it beside python.exe
    file(INSTALL "${CAIRO_DLL_RELEASE}" DESTINATION "${CURRENT_PYTHON3_PREFIX}" RENAME "libcairo-2${VCPKG_TARGET_SHARED_LIBRARY_SUFFIX}")
    vcpkg_copy_tool_dependencies("${CURRENT_PYTHON3_PREFIX}")

    # Any DLLs already copied by python.exe need to be scrubbed. Otherwise, the validation will fail
    # with a conflict.
    file(GLOB _PYTHON3_DIR_CONTENTS LIST_DIRECTORIES false RELATIVE "${INSTALLED_PYTHON3_PREFIX}" "${INSTALLED_PYTHON3_PREFIX}/*")
    foreach(i IN LISTS _PYTHON3_DIR_CONTENTS)
        set(_TEST_PATH "${CURRENT_PYTHON3_PREFIX}/${i}")
        if(EXISTS "${_TEST_PATH}")
            file(REMOVE "${_TEST_PATH}")
        endif()
    endforeach()
endif()

# Use pip to install cairosvg into the staging dir.
message(STATUS "Installing cairosvg from pip")
file(TO_NATIVE_PATH "${CURRENT_PYTHON3_PREFIX}" _PIP_PREFIX)
vcpkg_execute_required_process(
    COMMAND "${PYTHON3_EXECUTABLE}" -m pip install --prefix "${_PIP_PREFIX}" "cairosvg==${CAIROSVG_VERSION}"
    LOGNAME install
)

# Perform a sample import of cairosvg to ensure all this weird hacking actually works.
message(STATUS "Testing cairosvg")
set(ENV{PYTHONPATH} "${CURRENT_PYTHON3_PREFIX}/Lib/site-packages")
vcpkg_execute_required_process(
    COMMAND "${PYTHON3_EXECUTABLE}" -c "import cairosvg"
    WORKING_DIRECTORY "${CURRENT_PYTHON3_PREFIX}"
    LOGNAME test
)
