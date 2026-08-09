set(FLTX_BENCHMARK_COMPILE_GROWTH_COUNTS "25;100;200" CACHE STRING
    "Repeated-function counts used to measure expression compile growth.")
set(FLTX_BENCHMARK_EXPRESSION_SHAPE_COUNT 64 CACHE STRING
    "Repeated functions generated for each detailed f256 expression shape.")
set(FLTX_BENCHMARK_COMPILE_REPEATS 3 CACHE STRING
    "Number of compiler runs used to find each median compile time.")
if(MSVC)
    set(FLTX_BENCHMARK_CXX20_MAX_COMPILE_REGRESSION_PERCENT 10 CACHE STRING
        "Maximum permitted optimized f256-expression compile regression versus C++23.")
else()
    set(FLTX_BENCHMARK_CXX20_MAX_COMPILE_REGRESSION_PERCENT 15 CACHE STRING
        "Maximum permitted optimized f256-expression compile regression versus C++23.")
endif()
set(FLTX_BENCHMARK_CXX20_MAX_OBJECT_GROWTH_PERCENT 5 CACHE STRING
    "Maximum permitted optimized f256-expression object growth versus C++23.")

add_custom_target(fltx_compile_benchmarks
    COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/measure.py"
        --compiler "${CMAKE_CXX_COMPILER}"
        --compiler-id "${CMAKE_CXX_COMPILER_ID}"
        --include "${PROJECT_SOURCE_DIR}/include"
        --work "${CMAKE_CURRENT_BINARY_DIR}/summary_work"
        --output "${CMAKE_CURRENT_BINARY_DIR}/compile_summary.csv"
        --scope summary
        --growth-counts ${FLTX_BENCHMARK_COMPILE_GROWTH_COUNTS}
        --repeats "${FLTX_BENCHMARK_COMPILE_REPEATS}"
        --standards 20 23
        --max-compile-regression-percent
            "${FLTX_BENCHMARK_CXX20_MAX_COMPILE_REGRESSION_PERCENT}"
        --max-object-growth-percent
            "${FLTX_BENCHMARK_CXX20_MAX_OBJECT_GROWTH_PERCENT}"
    USES_TERMINAL
    VERBATIM
    COMMENT "Measuring fltx public-header and f256 expression compile times"
)

add_custom_target(fltx_expression_compile_matrix
    COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/measure.py"
        --compiler "${CMAKE_CXX_COMPILER}"
        --compiler-id "${CMAKE_CXX_COMPILER_ID}"
        --include "${PROJECT_SOURCE_DIR}/include"
        --work "${CMAKE_CURRENT_BINARY_DIR}/expression_work"
        --output "${CMAKE_CURRENT_BINARY_DIR}/expression_matrix.csv"
        --scope expressions
        --shape-count "${FLTX_BENCHMARK_EXPRESSION_SHAPE_COUNT}"
        --repeats "${FLTX_BENCHMARK_COMPILE_REPEATS}"
        --standards 20
    USES_TERMINAL
    VERBATIM
    COMMENT "Measuring 49 f256 expression shapes plus the aggregate fused-shape case"
)

add_custom_target(fltx_compile_benchmarks_full
    DEPENDS fltx_compile_benchmarks fltx_expression_compile_matrix
)
