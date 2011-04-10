add_definitions(-D_LIB)         # We shouldn't actually need this here, but
                                # Cyan uses it, so meh

add_subdirectory(plClient)
add_subdirectory(plPythonPack)
