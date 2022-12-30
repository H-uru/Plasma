# Some optional dependencies are included as git submodule, and there's a
# chance the repo wasn't cloned with submodules recursively. In that case, we
# want to try to automatically initialize them.
function(plasma_init_git_submodule MODULE)
    cmake_parse_arguments(PARSE_ARGV 1 _pigs "" "RESULT" "")

    if(EXISTS "${CMAKE_SOURCE_DIR}/${MODULE}/.git")
        set(return_value TRUE)
    else()
        # First, we need to see if we have git available...
        find_package(Git QUIET)

        if(Git_FOUND)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${MODULE}
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                RESULT_VARIABLE RETURNCODE
            )

            if(RETURNCODE)
                set(return_value FALSE)
            else()
                set(return_value TRUE)
            endif()
        else()
            message("Could not automatically initialize git submodule at ${MODULE}")
            set(return_value FALSE)
        endif()
    endif()

    if(_pigs_RESULT)
        set(${_pigs_RESULT} ${return_value} PARENT_SCOPE)
    endif()
endfunction()
