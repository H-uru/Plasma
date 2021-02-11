add_executable(plLocalizationBenchmark EXCLUDE_FROM_ALL main.cpp)
target_link_libraries(
    plLocalizationBenchmark
    PRIVATE
        CoreLib
        pfLocalizationMgr
        string_theory
)
