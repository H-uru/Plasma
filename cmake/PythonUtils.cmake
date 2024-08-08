option(ENABLE_PIP_INSTALL "Allow automatic installation of Python dependency modules." ${USE_VCPKG})

function(python_test_modules)
    cmake_parse_arguments(_ptm "DONT_INSTALL;REQUIRED" "REQUIREMENTS_FILE" "MODULES" ${ARGN})

    if(NOT _ptm_MODULES)
        message(FATAL_ERROR "python_test_modules() must be given a list of MODULES")
    endif()

    # Optional: attempt to install modules from pip before checking their existence.
    if(ENABLE_PIP_INSTALL AND NOT _ptm_DONT_INSTALL)
        if(NOT _PLASMA_ENSUREPIP_DONE)
            message(STATUS "Bootstrapping pip...")
            execute_process(
                COMMAND "${Python3_EXECUTABLE}" -m ensurepip
                RESULT_VARIABLE RETURNCODE
                OUTPUT_QUIET ERROR_QUIET
            )
            if(RETURNCODE)
                message(WARNING "Unable to bootstrap pip")
            endif()
            set(_PLASMA_ENSUREPIP_DONE TRUE CACHE INTERNAL "" FORCE)
        endif()

        if(_ptm_REQUIREMENTS_FILE)
            # Optimization: hash the requirements file and store it on success
            # Then, we can avoid running expensive install operations on reconfigure.
            file(SHA512 "${_ptm_REQUIREMENTS_FILE}" _REQUIREMENTS_FILE_HASH)
            if(NOT _REQUIREMENTS_FILE_HASH IN_LIST _PLASMA_PIP_REQUIREMENTS_HASHES)
                message(STATUS "Installing Python modules from: ${_ptm_REQUIREMENTS_FILE}")
                execute_process(
                    COMMAND "${Python3_EXECUTABLE}" -m pip install -r "${_ptm_REQUIREMENTS_FILE}"
                    RESULT_VARIABLE RETURNCODE
                    OUTPUT_QUIET ERROR_QUIET
                )
                if(RETURNCODE EQUAL 0)
                    set(_PLASMA_PIP_REQUIREMENTS_HASHES "${_PLASMA_PIP_REQUIREMENTS_HASHES}${_REQUIREMENTS_FILE_HASH};" CACHE INTERNAL "")
                endif()
            else()
                message(STATUS "Already installed Python modules from: ${_ptm_REQUIREMENTS_FILE}")
            endif()
        else()
            message(STATUS "Installing Python modules: ${_ptm_MODULES}")
            execute_process(
                COMMAND "${Python3_EXECUTABLE}" -m pip install ${_ptm_MODULES}
                OUTPUT_QUIET ERROR_QUIET
            )
        endif()

        if(RETURNCODE)
            message(WARNING "Failed to install modules")
        endif()
    endif()

    foreach(_module_NAME IN LISTS _ptm_MODULES)
        if(Python3_EXECUTABLE)
            execute_process(
                COMMAND "${Python3_EXECUTABLE}" -c "import ${_module_NAME}"
                RESULT_VARIABLE RETURNCODE
                OUTPUT_QUIET ERROR_QUIET
            )
            if(NOT RETURNCODE)
                set(${_module_NAME}_FOUND TRUE PARENT_SCOPE)
            else()
                set(${_module_NAME}_FOUND FALSE PARENT_SCOPE)
            endif()
        else()
            set(${_module_NAME}_FOUND FALSE PARENT_SCOPE)
        endif()

        if(_ptm_REQUIRED AND NOT ${_module_NAME}_FOUND)
            message(FATAL_ERROR "Could NOT find Python3 Module: ${_module_NAME}")
        endif()
    endforeach()
endfunction()
