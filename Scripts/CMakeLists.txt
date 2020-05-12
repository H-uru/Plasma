add_custom_target(Scripts ALL
                  COMMENT "Copying engine scripts..."

                  # Copy core engine files
                  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/dat" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dat"
                  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Python" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/python"
                  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/SDL" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/SDL"
)

# These Python stdlib modules should be EXCLUDED from the copy due to either practical or
# security concerns.
set(Scripts_BLACKLIST
    ^__pycache__
    ^asyncio
    ^ctypes
    ^curses
    ^distutils
    ^dist-packages
    ^email
    ^ensurepip
    ^http
    # contains a module-level SystemExit
    ^idlelib
    ^lib2to3
    ^msilib
    # Windows turd
    ^site-packages
    ^test
    ^tkinter
    ^turtledemo
    ^unittest
    ^urllib
    ^venv
    ^wsgiref
    # annoying browser popup POS
    ^antigravity.py$
    ^asynchat.py$
    ^asyncore.py$
    ^socket.py$
    ^socketserver.py$
)

# Hunt down all applicable stdlib modules and copy them over.
file(GLOB_RECURSE Scripts_SYSTEM LIST_DIRECTORIES false RELATIVE "${Python3_STDARCH}" "${Python3_STDARCH}/*.py")
foreach(i ${Scripts_BLACKLIST})
    list(FILTER Scripts_SYSTEM EXCLUDE REGEX ${i})
endforeach()

add_custom_target(ScriptsSystem
                  COMMENT "Copying system scripts...")
foreach(i ${Scripts_SYSTEM})
    add_custom_command(TARGET ScriptsSystem POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy "${Python3_STDARCH}/${i}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Python/system/${i}"
    )
endforeach()
