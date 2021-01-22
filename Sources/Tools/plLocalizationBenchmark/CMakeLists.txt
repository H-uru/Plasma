add_executable(plLocalizationBenchmark main.cpp)
target_link_libraries(
    plLocalizationBenchmark
    PRIVATE
        CoreLib
        pfLocalizationMgr
        string_theory
)
