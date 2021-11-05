plasma_executable(plSystemInfo TOOL SOURCES main.cpp)
target_link_libraries(
    plSystemInfo
    PRIVATE
        CoreLib
)
