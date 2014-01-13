if( MSVC ) # VS2012 doesn't support correctly the tuples yet and gtest doesn't compile without this
    add_definitions( /D _VARIADIC_MAX=10 )
endif()

add_subdirectory(gtest-1.6.0)

add_subdirectory(CoreLibTests)
#add_subdirectory(FeatureTests)
add_subdirectory(NucleusTests)
#add_subdirectory(PubUtilTests)
