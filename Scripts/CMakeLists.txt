file(GLOB_RECURSE GameScripts "${CMAKE_CURRENT_SOURCE_DIR}/Python/*.py")
file(GLOB_RECURSE GameSDL "${CMAKE_CURRENT_SOURCE_DIR}/SDL/*.sdl")
file(GLOB_RECURSE GameData "${CMAKE_CURRENT_SOURCE_DIR}/dat/*.age" "${CMAKE_CURRENT_SOURCE_DIR}/dat/*.fni")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Python/" PREFIX "Python" FILES ${GameScripts})
source_group("SDL" FILES ${GameSDL})
source_group("Ages" FILES ${GameData})

add_custom_target(Scripts
                  SOURCES ${GameScripts} ${GameSDL} ${GameData}
)
set_target_properties(Scripts PROPERTIES FOLDER "")

install(
    DIRECTORY "Python/"
    DESTINATION client/python
    FILES_MATCHING PATTERN "*.py"
)
install(
    DIRECTORY "SDL/"
    DESTINATION client/SDL
    FILES_MATCHING PATTERN "*.sdl"
)
install(
    DIRECTORY "dat/"
    DESTINATION client/dat
    FILES_MATCHING PATTERN "*.age" PATTERN "*.fni"
)

file(TO_CMAKE_PATH "${Python3_STDARCH}" Scripts_SYSTEM_PATH)
install(
    DIRECTORY "${Scripts_SYSTEM_PATH}/"
    DESTINATION client/python/system

    FILES_MATCHING
    PATTERN "*.py"
    PATTERN "lib-dynload/*"
    PATTERN "*.pyc" EXCLUDE

    # These Python stdlib modules should be EXCLUDED from the copy due to either practical or
    # security concerns. NOTE: When adding a new PATTERN, be sure to "quote" the directory or file
    # name, otherwise you will break the install command in a silent, yet disastrous way.
    PATTERN "__pycache__" EXCLUDE
    PATTERN "asyncio" EXCLUDE
    PATTERN "ctypes" EXCLUDE
    PATTERN "curses" EXCLUDE
    PATTERN "distutils" EXCLUDE
    PATTERN "dist-packages" EXCLUDE
    PATTERN "email" EXCLUDE
    PATTERN "ensurepip" EXCLUDE
    PATTERN "http" EXCLUDE
    # contains a module-level SystemExit
    PATTERN "idlelib" EXCLUDE
    PATTERN "lib2to3" EXCLUDE
    PATTERN "msilib" EXCLUDE
    # Windows turd
    PATTERN "site-packages" EXCLUDE
    PATTERN "test" EXCLUDE
    PATTERN "tkinter" EXCLUDE
    PATTERN "turtledemo" EXCLUDE
    PATTERN "unittest" EXCLUDE
    PATTERN "urllib" EXCLUDE
    PATTERN "venv" EXCLUDE
    PATTERN "wsgiref" EXCLUDE

    # Annoying browser popup turd.
    PATTERN "antigravity.py" EXCLUDE
    PATTERN "asynchat.py" EXCLUDE
    PATTERN "asyncore.py" EXCLUDE
    PATTERN "socket.py" EXCLUDE
    PATTERN "socketserver.py" EXCLUDE
)
