plasma_executable(plSystemInfo TOOL
    FOLDER Tools
    SOURCES main.cpp
)
target_link_libraries(
    plSystemInfo
    PRIVATE
        CoreLib
)
