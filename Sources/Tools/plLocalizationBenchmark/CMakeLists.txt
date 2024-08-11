plasma_executable(plLocalizationBenchmark
    FOLDER Tools
    EXCLUDE_FROM_ALL
    SOURCES main.cpp
)
target_link_libraries(
    plLocalizationBenchmark
    PRIVATE
        CoreLib
        pfLocalizationMgr
        string_theory
)
