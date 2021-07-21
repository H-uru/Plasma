# This port doesn't include an "include" directory or a "copyright" file, so it's considered
# empty by vcpkg. This will fail the validation unless we mark the port empty.
set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
if(NOT VCPKG_TARGET_IS_WINDOWS)
    message(STATUS "Install cairo on Ubuntu with `sudo apt install libcairo2` and on macOS with `brew install cairo`.")
endif()

set(CAIROSVG_VERSION "2.5.1")
set(INSTALLED_PYTHON_PREFIX "${CURRENT_INSTALLED_DIR}/tools/python3")

# We are running in script mode, so no toolchains are available. Sad.
find_program(
    PYTHON_EXECUTABLE
    NAMES python python3 python3.9
    PATHS "${INSTALLED_PYTHON_PREFIX}"
    NO_DEFAULT_PATH
)

if(EXISTS CURRENT_PACKAGES_DIR)
    file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}")
endif()

# Figure out from Python what its roots are in a mostly platform independent way. For Windows,
# this should be the tools/python3 directory. In other cases, it should be CURRENT_INSTALLED_DIR.
vcpkg_execute_in_download_mode(
    COMMAND "${PYTHON_EXECUTABLE}" -c "import sys;sys.stdout.write(sys.exec_prefix)"
    WORKING_DIRECTORY "${INSTALLED_PYTHON_PREFIX}"
    OUTPUT_VARIABLE INSTALLED_PYTHON_PREFIX
)
vcpkg_execute_in_download_mode(
    COMMAND "${PYTHON_EXECUTABLE}" -c "import sys,distutils.sysconfig;sys.stdout.write(distutils.sysconfig.get_python_lib(plat_specific=False,standard_lib=False))"
    WORKING_DIRECTORY "${INSTALLED_PYTHON_PREFIX}"
    OUTPUT_VARIABLE INSTALLED_SITE_PACKAGES_DIR
)

# The returned values from above are absolute paths into the installed tree. However, we cannot
# use those right now because ports are installed into a staging directory. Therefore, we strip
# the prefix off of site-packages and root it onto the current staging dir..............
# Can't use CMake's relative pathing because it requires file paths, not directory paths. Grr.
function(get_relative_path RESULT_VAR ROOT_PREFIX CHILD_PATH)
    if(NOT ROOT_PREFIX OR NOT CHILD_PATH)
        message(FATAL_ERROR "Got an empty path: '${ROOT_PREFIX}' not in '${CHILD_PATH}'")
    endif()

    file(TO_CMAKE_PATH "${ROOT_PREFIX}" ROOT_PREFIX)
    file(TO_CMAKE_PATH "${CHILD_PATH}" CHILD_PATH)

    string(FIND "${CHILD_PATH}" "${ROOT_PREFIX}" _FIND_ASS)
    if(NOT _FIND_ASS EQUAL 0)
        message(FATAL_ERROR "Relative path error: '${ROOT_PREFIX}' not in '${CHILD_PATH}'")
    endif()

    string(LENGTH "${ROOT_PREFIX}" _PREFIX_LENGTH)
    math(EXPR _PREFIX_LENGTH "${_PREFIX_LENGTH} + 1")
    string(LENGTH "${CHILD_PATH}" _CHILD_LENGTH)

    if(_CHILD_LENGTH GREATER_EQUAL _PREFIX_LENGTH)
        string(SUBSTRING "${CHILD_PATH}" ${_PREFIX_LENGTH} -1 _RESULT)
    endif()
    set(${RESULT_VAR} "${_RESULT}" PARENT_SCOPE)
endfunction()

function(join_path RESULT_VAR)
    # Avoid directly string(JOIN ...) in case one of the path elements is empty
    foreach(i IN LISTS ARGN)
        if(i)
            if(_RESULT)
                string(APPEND _RESULT "/")
            endif()
            string(APPEND _RESULT "${i}")
        endif()
    endforeach()
    if(_RESULT)
        file(TO_CMAKE_PATH "${_RESULT}" _RESULT)
    endif()
    set(${RESULT_VAR} "${_RESULT}" PARENT_SCOPE)
endfunction()

get_relative_path(_PYTHON_DIR "${CURRENT_INSTALLED_DIR}" "${INSTALLED_PYTHON_PREFIX}")
get_relative_path(_SITE_PACKAGES_DIR "${INSTALLED_PYTHON_PREFIX}" "${INSTALLED_SITE_PACKAGES_DIR}")
join_path(CURRENT_PYTHON_PREFIX "${CURRENT_PACKAGES_DIR}" "${_PYTHON_DIR}")
join_path(CURRENT_SITE_PACKAGES_DIR "${CURRENT_PACKAGES_DIR}" "${_PYTHON_DIR}" "${_SITE_PACKAGES_DIR}")

# Copy over cairo.dll as libcairo-2.dll on Windows only. Other OSes should install cairo from
# the system package manager. This will be tested by the non-platform specific bits.
if(VCPKG_TARGET_IS_WINDOWS)
    list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES ${VCPKG_TARGET_SHARED_LIBRARY_SUFFIX})
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
    file(INSTALL "${CAIRO_DLL_RELEASE}" DESTINATION "${CURRENT_PYTHON_PREFIX}" RENAME "libcairo-2${VCPKG_TARGET_SHARED_LIBRARY_SUFFIX}")
    vcpkg_copy_tool_dependencies("${CURRENT_PYTHON_PREFIX}")

    # Any DLLs already copied by python.exe need to be scrubbed. Otherwise, the validation will fail
    # with a conflict.
    file(GLOB _PYTHON_DIR_CONTENTS LIST_DIRECTORIES false RELATIVE "${INSTALLED_PYTHON_PREFIX}" "${INSTALLED_PYTHON_PREFIX}/*")
    foreach(i IN LISTS _PYTHON_DIR_CONTENTS)
        set(_TEST_PATH "${CURRENT_PYTHON_PREFIX}/${i}")
        if(EXISTS "${_TEST_PATH}")
            file(REMOVE "${_TEST_PATH}")
        endif()
    endforeach()
endif()

# Use pip to install cairosvg into the staging dir.
message(STATUS "Installing cairosvg from pip")
file(TO_NATIVE_PATH "${CURRENT_PYTHON_PREFIX}" _PIP_PREFIX)
vcpkg_execute_required_process(
    COMMAND "${PYTHON_EXECUTABLE}" -m pip install --prefix "${_PIP_PREFIX}" "cairosvg==${CAIROSVG_VERSION}"
    ALLOW_IN_DOWNLOAD_MODE
    WORKING_DIRECTORY "${CURRENT_PYTHON_PREFIX}"
    LOGNAME install
)
# Perform a sample import of cairosvg to ensure all this weird hacking actually works.
message(STATUS "Testing cairosvg")
set(ENV{PYTHONPATH} "${CURRENT_SITE_PACKAGES_DIR}")
vcpkg_execute_required_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c "import cairosvg"
    ALLOW_IN_DOWNLOAD_MODE
    WORKING_DIRECTORY "${CURRENT_PYTHON_PREFIX}"
    LOGNAME test
)
