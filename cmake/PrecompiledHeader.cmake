if(MSVC)
    set(PCH_SUPPORTED TRUE)
else()
    set(PCH_SUPPORTED FALSE)
endif()

if(PCH_SUPPORTED)
    option(PLASMA_USE_PCH "Enable precompiled headers?" ON)
endif(PCH_SUPPORTED)

function(use_precompiled_header PrecompiledHeader PrecompiledSource Headers Sources)
    if(PLASMA_USE_PCH)
        if(MSVC)
            get_filename_component(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
            set(PrecompiledBinary ${PrecompiledBasename}.pch)

            add_definitions(/Yu"${PrecompiledHeader}")
            add_definitions(/FI"${PrecompiledHeader}")
            set_source_files_properties(${PrecompiledSource} PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\"")
        endif(MSVC)

        # Add the Pch.[h|cpp] to the appropriate sets
        # OT: This has to be the oddest thing I've ever written =/
        set(${Headers} ${${Headers}} ${PrecompiledHeader} PARENT_SCOPE)
        set(${Sources} ${${Sources}} ${PrecompiledSource} PARENT_SCOPE)
    endif(PLASMA_USE_PCH)
endfunction(use_precompiled_header)
