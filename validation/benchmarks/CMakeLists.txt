add_subdirectory(compile)
add_subdirectory(binary_size)

if(BUILD_TESTING)
    add_test(
        NAME fltx.benchmarks.compile.tools
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/compile/test_measure.py"
    )
    set_tests_properties(fltx.benchmarks.compile.tools PROPERTIES
        LABELS "fltx;metrics;compile"
    )
endif()
