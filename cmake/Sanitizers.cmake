if(CMAKE_VS_DEVENV_COMMAND)
    set(_vs_install_root "${CMAKE_VS_DEVENV_COMMAND}/../../../")
    get_filename_component(_vs_install_root "${_vs_install_root}" ABSOLUTE)
    set(_llvm_dir "${_vs_install_root}/VC/Tools/Llvm")
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "AMD64|x86-64")
        list(APPEND _llvm_bin "${_llvm_dir}/x64/bin")
    endif()
    list(APPEND _llvm_bin "${_llvm_dir}/bin")
endif()

find_program(CLANG_APPLY_EXE NAMES "clang-apply-replacements" PATHS ${_llvm_bin} DOC "Path to clang-apply-replacements executable")
find_program(CLANG_TIDY_EXE NAMES "clang-tidy" PATHS ${_llvm_bin} DOC "Path to clang-tidy executable")

if(CMAKE_GENERATOR MATCHES "Makefile|Ninja")
    set(USE_SANITIZE_TARGETS TRUE)
    # Default such that we don't clobber PlasmaTargets.
    set(SANITIZE_DEFAULT_VALUE OFF)
else()
    set(SANITIZE_DEFAULT_VALUE ON)
endif()

# Conditional that evaluates to whether or not build optimizations are allowed.
# This is due to the sanitizers needing the compile commands of each file, not
# the unity files. CMake will generate files like cmake_pch.cxx, and unity_0.cxx.
# Both of these files are listed in the compile commands JSON, however, with
# the current tidy target implementation, the sanitizer cannot know about them.
# This results in bad/corrupted PCH errors and missing compile commands (errors)
# for source files included in unity builds.
if(USE_SANITIZE_TARGETS AND USE_CLANG_TIDY)
    set(ALLOW_BUILD_OPTIMIZATIONS FALSE)
else()
    set(ALLOW_BUILD_OPTIMIZATIONS TRUE)
endif()

cmake_dependent_option(
    USE_CLANG_TIDY
    "Allow the codebase to use the clang-tidy sanitizer"
    ${SANITIZE_DEFAULT_VALUE}
    "CLANG_TIDY_EXE"
    OFF
)

if(USE_CLANG_TIDY)
    # Don't set CMAKE_CXX_CLANG_TIDY. CMake+Ninja+MSVC+clang-tidy are broken in this mode, see:
    # https://gitlab.kitware.com/cmake/cmake/-/issues/20512
    # https://bugs.llvm.org/show_bug.cgi?id=45356
    # Use the tidy target instead.
    if(USE_SANITIZE_TARGETS)
        set(_TIDY_REPLACEMENTS_DIR "${CMAKE_BINARY_DIR}/clang-tidy-replacements")
        add_custom_target(tidy_init
            COMMENT "Preparing to tidy..."
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${_TIDY_REPLACEMENTS_DIR}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${_TIDY_REPLACEMENTS_DIR}"
        )
        add_custom_target(tidy DEPENDS tidy_init)
        if(CLANG_APPLY_EXE)
            add_custom_target(fix
                DEPENDS tidy
                COMMENT "Applying fixes..."
                COMMAND ${CLANG_APPLY_EXE} "${_TIDY_REPLACEMENTS_DIR}"
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            )
        endif()

        # If we were on CMake 3.20, we could do this per-target...
        set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    endif()
endif()

function(plasma_sanitize_target TARGET)
    if(USE_CLANG_TIDY)
        # CMake only natively supports CXX_CLANG_TIDY with Unix Makefiles and Ninja generator as of
        # CMake 3.19. But most everyone is using Visual Studio, so we'll reset the code analysis
        # type to clang-tidy. This is a huge win given how terrible MS's code analysis is.
        if(CMAKE_GENERATOR MATCHES "Visual Studio")
            set_target_properties(
                ${TARGET} PROPERTIES
                VS_GLOBAL_ClangTidyToolExe "${CLANG_TIDY_EXE}"
                VS_GLOBAL_EnableClangTidyCodeAnalysis TRUE
                VS_GLOBAL_EnableMicrosoftCodeAnalysis FALSE
            )
        endif()

        if(USE_SANITIZE_TARGETS)
            get_target_property(_TARGET_SOURCES ${TARGET} SOURCES)
            get_target_property(_TARGET_SOURCE_DIR ${TARGET} SOURCE_DIR)
            foreach(_SOURCE_FILE IN LISTS _TARGET_SOURCES)
                # Prevent CMake from stoopidly feeding Python files and bitmaps to clang-tidy.
                # Yes, CMake actually does try to do that...
                if(_SOURCE_FILE MATCHES [[\.c$|\.cpp$|\.cxx$|\.cc$]])
                    list(APPEND _TIDY_SOURCES "${_TARGET_SOURCE_DIR}/${_SOURCE_FILE}")
                endif()
            endforeach()

            # clang-tidy gives an error if no source files are passed,
            # so create the target only if there are any source files.
            # This is relevant for targets that only contain header files, such as pfFeatureInc.
            if(_TIDY_SOURCES)
                set(_TARGET_FIXES_FILE "${_TIDY_REPLACEMENTS_DIR}/${TARGET}.yaml")
                add_custom_target(tidy_${TARGET}
                    DEPENDS tidy_init
                    COMMENT "Running clang-tidy on ${TARGET}..."
                    COMMAND_EXPAND_LISTS
                    COMMAND ${CLANG_TIDY_EXE} --quiet -p "${CMAKE_BINARY_DIR}" "-export-fixes=${_TARGET_FIXES_FILE}" "${_TIDY_SOURCES}"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    BYPRODUCTS "${_TARGET_FIXES_FILE}"
                )
                add_dependencies(tidy tidy_${TARGET})
            endif()
        endif()
    endif()
endfunction()
