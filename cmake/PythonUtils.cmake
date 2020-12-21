# TODO: Eventually, we want to automatically turn on this auto-pip install functionality. That
# should only be done when we KNOW that we are in an isolated configuration of Python.
option(ENABLE_PIP_INSTALL "Allow automatic installation of Python dependency modules." OFF)

function(python_test_modules)
    cmake_parse_arguments(_ptm "REQUIRED" "" "MODULES" ${ARGN})

    if(NOT _ptm_MODULES)
        message(FATAL_ERROR "python_test_modules() must be given a list of MODULES")
    endif()

    foreach(_module_NAME IN LISTS _ptm_MODULES)
        if(Python3_EXECUTABLE)
            execute_process(
                COMMAND "${Python3_EXECUTABLE}" -c "import ${_module_NAME}"
                RESULT_VARIABLE RETURNCODE
                OUTPUT_QUIET ERROR_QUIET
            )
            if(NOT RETURNCODE)
                set(${_module_NAME}_FOUND ON PARENT_SCOPE)
            else()
                set(${_module_NAME}_FOUND OFF PARENT_SCOPE)
            endif()
        else()
            set(${_module_NAME}_FOUND OFF PARENT_SCOPE)
        endif()

        if(_ptm_REQUIRED AND NOT ${_module_NAME}_FOUND)
            message(FATAL_ERROR "Could NOT find Python3 Module: ${_module_NAME}")
        endif()
    endforeach()
endfunction()

function(python_install_modules)
    cmake_parse_arguments(_pim "" "REQUIREMENTS_FILE" "MODULES" ${ARGN})

    if(NOT _pim_REQUIREMENTS_FILE AND NOT _pim_MODULES)
        message(FATAL_ERROR "python_install_modules() requires either a REQUIREMENTS_FILE or a list of MODULES")
    endif()
    if(NOT Python3_EXECUTABLE)
        message(WARNING "Python3 is not available, not installing requested modules.")
    endif()

    # Escape hatch for folks who don't want us mucking around with their system Python.
    if(NOT ENABLE_PIP_INSTALL)
        return()
    endif()

    if(NOT _PLASMA_ENSUREPIP_DONE)
        message(STATUS "Bootstrapping pip...")
        execute_process(
            COMMAND "${Python3_EXECUTABLE}" -m ensurepip
            RESULT_VARIABLE RETURNCODE
            OUTPUT_QUIET ERROR_QUIET
        )
        if(RETURNCODE)
            message(WARNING "Unable to bootstrap pip")
            return()
        endif()
        set(_PLASMA_ENSUREPIP_DONE ON CACHE INTERNAL "" FORCE)
    endif()

    if(_pim_REQUIREMENTS_FILE)
        message(STATUS "Installing Python modules from: ${_pim_REQUIREMENTS_FILE}")
        execute_process(
            COMMAND "${Python3_EXECUTABLE}" -m pip install -r "${_pim_REQUIREMENTS_FILE}"
            RESULT_VARIABLE RETURNCODE
            OUTPUT_QUIET ERROR_QUIET
        )
        if(RETURNCODE)
            message(WARNING "Failed to install modules")
            unset(RETURNCODE)
        endif()
    endif()

    if(_pim_MODULES)
        message(STATUS "Installing Python modules: ${_pim_MODULES}")
        execute_process(
            COMMAND "${Python3_EXECUTABLE}" -m pip install ${_pim_MODULES}
            OUTPUT_QUIET ERROR_QUIET
        )
        if(RETURNCODE)
            message(WARNING "Failed to install modules")
        endif()
    endif()
endfunction()
