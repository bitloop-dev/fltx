# Accuracy, benchmark, source-identity, and smoke-runner targets.

set(FLTX_ACCURACY_SOURCE_FILE_PATHS
    "${FLTX_VALIDATION_SOURCE_DIR}/accuracy/main.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/accuracy/f128.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/accuracy/f256.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/accuracy/native.cpp"
)

function(fltx_tests_add_accuracy_runner
    target library simulated_consteval consumer_fast_math)
    add_executable(${target} ${FLTX_ACCURACY_SOURCE_FILE_PATHS})
    fltx_tests_prepare_runner(${target} ${library})
    fltx_tests_configure_consumer_mode(${target} "${consumer_fast_math}")
    target_compile_definitions(${target} PRIVATE
        "FLTX_ACCURACY_EXPECTS_SIMULATED_CONSTEVAL=${simulated_consteval}"
    )
endfunction()

fltx_tests_add_accuracy_runner(fltx_accuracy fltx::fltx 0 OFF)
fltx_tests_add_accuracy_runner(
    fltx_constexpr_accuracy
    fltx_constexpr_accuracy_lib
    1
    OFF
)
fltx_tests_add_accuracy_runner(fltx_accuracy_fastmath fltx::fltx 0 ON)
fltx_tests_add_accuracy_runner(
    fltx_constexpr_accuracy_fastmath
    fltx_constexpr_accuracy_lib
    1
    ON
)

set(FLTX_ACCURACY_TARGETS
    fltx_accuracy
    fltx_constexpr_accuracy
    fltx_accuracy_fastmath
    fltx_constexpr_accuracy_fastmath
)
set(FLTX_RUNTIME_ACCURACY_TARGETS
    fltx_accuracy
    fltx_accuracy_fastmath
)
set(FLTX_CONSTEXPR_ACCURACY_TARGETS
    fltx_constexpr_accuracy
    fltx_constexpr_accuracy_fastmath
)
foreach(target IN LISTS FLTX_ACCURACY_TARGETS)
    target_include_directories(${target} PRIVATE
        "${FLTX_TESTS_MPFR_INCLUDE_DIR}"
        "${FLTX_TESTS_GMP_INCLUDE_DIR}"
    )
    target_link_libraries(${target} PRIVATE
        "${FLTX_TESTS_MPFR_LIBRARY}"
        "${FLTX_TESTS_GMP_LIBRARY}"
    )
endforeach()

foreach(target IN LISTS FLTX_RUNTIME_ACCURACY_TARGETS)
    if(FLTX_METRICS_TLFLOAT)
        target_link_libraries(${target} PRIVATE fltx::test_tlfloat)
        target_compile_definitions(${target} PRIVATE FLTX_METRICS_HAS_TLFLOAT=1)
    else()
        target_compile_definitions(${target} PRIVATE FLTX_METRICS_HAS_TLFLOAT=0)
    endif()
endforeach()

# Comparison-library rows are publication metadata, not constexpr validation.
# Keep the fixed runner focused on FLTX while retaining the same MPFR oracle,
# domains, special-value contracts, and thresholds.
foreach(target IN LISTS FLTX_CONSTEXPR_ACCURACY_TARGETS)
    target_compile_definitions(${target} PRIVATE FLTX_METRICS_HAS_TLFLOAT=0)
endforeach()

set(FLTX_BENCHMARK_SOURCE_FILE_PATHS
    "${FLTX_VALIDATION_SOURCE_DIR}/benchmarks/runtime/main.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/benchmarks/runtime/operations.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/benchmarks/runtime/workloads.cpp"
)

function(fltx_tests_add_benchmark_runner target consumer_fast_math)
    add_executable(${target} ${FLTX_BENCHMARK_SOURCE_FILE_PATHS})
    fltx_tests_prepare_runner(${target})
    fltx_tests_configure_consumer_mode(${target} "${consumer_fast_math}")
    target_include_directories(${target} PRIVATE
        "${FLTX_TESTS_MPFR_INCLUDE_DIR}"
        "${FLTX_TESTS_GMP_INCLUDE_DIR}"
    )
    target_link_libraries(${target} PRIVATE
        "${FLTX_TESTS_MPFR_LIBRARY}"
        "${FLTX_TESTS_GMP_LIBRARY}"
    )
    if(FLTX_METRICS_TLFLOAT)
        target_link_libraries(${target} PRIVATE fltx::test_tlfloat)
        target_compile_definitions(${target} PRIVATE FLTX_METRICS_HAS_TLFLOAT=1)
    else()
        target_compile_definitions(${target} PRIVATE FLTX_METRICS_HAS_TLFLOAT=0)
    endif()
endfunction()

fltx_tests_add_benchmark_runner(fltx_benchmark OFF)
fltx_tests_add_benchmark_runner(fltx_benchmark_fastmath ON)

set(FLTX_BENCHMARK_TARGETS fltx_benchmark fltx_benchmark_fastmath)

set(FLTX_METRICS_BUNDLED_QDPP_INCLUDE_DIR
    "${FLTX_VALIDATION_SOURCE_DIR}/extern/qdpp/include"
)
option(FLTX_METRICS_QDPP
    "Build qdpp-backed accuracy and benchmark comparisons."
    ON
)
set(FLTX_METRICS_QDPP_INCLUDE_DIR
    "${FLTX_METRICS_BUNDLED_QDPP_INCLUDE_DIR}"
    CACHE PATH "qdpp include directory used by fltx metrics."
)
if(FLTX_METRICS_QDPP_INCLUDE_DIR STREQUAL "")
    set(FLTX_METRICS_QDPP_INCLUDE_DIR
        "${FLTX_METRICS_BUNDLED_QDPP_INCLUDE_DIR}"
        CACHE PATH "qdpp include directory used by fltx metrics."
        FORCE
    )
endif()

set(FLTX_METRICS_QDPP_CONTRACT_TARGET)
if(FLTX_METRICS_QDPP)
    if(EXISTS "${FLTX_METRICS_QDPP_INCLUDE_DIR}/qd/dd.h" AND
       EXISTS "${FLTX_METRICS_QDPP_INCLUDE_DIR}/qd/qd_real.h")
        foreach(target fltx_accuracy fltx_benchmark)
            target_include_directories(${target} PRIVATE
                "${FLTX_METRICS_QDPP_INCLUDE_DIR}"
            )
            target_compile_definitions(${target} PRIVATE
                FLTX_METRICS_HAS_QDPP=1
                FLTX_METRICS_QDPP_ENABLED=1
            )
        endforeach()

        # Consumer-fast-math reports intentionally contain FLTX rows only.
        # Do not parse qdpp headers in those translation units: qdpp rejects
        # aggressive fast-math at compile time even when its rows are disabled.
        foreach(target fltx_accuracy_fastmath fltx_benchmark_fastmath)
            target_compile_definitions(${target} PRIVATE
                FLTX_METRICS_HAS_QDPP=0
                FLTX_METRICS_QDPP_ENABLED=1
            )
        endforeach()

        add_executable(fltx_qdpp_smoke
            "${FLTX_VALIDATION_SOURCE_DIR}/support/qdpp_smoke.cpp"
        )
        target_include_directories(fltx_qdpp_smoke PRIVATE
            "${FLTX_METRICS_QDPP_INCLUDE_DIR}"
        )
        target_compile_features(fltx_qdpp_smoke PRIVATE cxx_std_20)
        set(FLTX_METRICS_QDPP_CONTRACT_TARGET fltx_qdpp_smoke)

        if(BUILD_TESTING)
            add_test(NAME fltx.qdpp.smoke COMMAND fltx_qdpp_smoke)
            set_tests_properties(fltx.qdpp.smoke PROPERTIES
                LABELS "fltx;metrics;qdpp"
            )
        endif()

        message(STATUS
            "fltx metrics: qdpp dependency enabled from "
            "${FLTX_METRICS_QDPP_INCLUDE_DIR}."
        )
    else()
        message(FATAL_ERROR
            "FLTX_METRICS_QDPP is ON, but FLTX_METRICS_QDPP_INCLUDE_DIR does not "
            "contain qd/dd.h and qd/qd_real.h."
        )
    endif()
else()
    foreach(target IN LISTS FLTX_RUNTIME_ACCURACY_TARGETS FLTX_BENCHMARK_TARGETS)
        target_compile_definitions(${target} PRIVATE
            FLTX_METRICS_HAS_QDPP=0
            FLTX_METRICS_QDPP_ENABLED=0
        )
    endforeach()
    message(STATUS
        "fltx metrics: qdpp explicitly disabled; Boost comparisons remain enabled."
    )
endif()
foreach(target IN LISTS FLTX_CONSTEXPR_ACCURACY_TARGETS)
    target_compile_definitions(${target} PRIVATE
        FLTX_METRICS_HAS_QDPP=0
        FLTX_METRICS_QDPP_ENABLED=0
    )
endforeach()

# Recompute this header on every metrics build. The Python helper only touches
# it when the source identity changes, so normal incremental builds stay cheap
# while a stale runner cannot masquerade as a newer checkout.
set(FLTX_METRICS_SOURCE_IDENTITY_DIR
    "${FLTX_VALIDATION_BINARY_DIR}/generated/source_identity"
)
set(FLTX_METRICS_SOURCE_IDENTITY_HEADER
    "${FLTX_METRICS_SOURCE_IDENTITY_DIR}/fltx_source_identity.generated.hpp"
)
add_custom_target(fltx_source_identity
    COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/validation/metrics/_internal/source_fingerprint.py"
        --root "${PROJECT_SOURCE_DIR}"
        --output "${FLTX_METRICS_SOURCE_IDENTITY_HEADER}"
    BYPRODUCTS "${FLTX_METRICS_SOURCE_IDENTITY_HEADER}"
    COMMENT "Refreshing fltx source fingerprint"
    VERBATIM
)
foreach(target IN LISTS FLTX_ACCURACY_TARGETS FLTX_BENCHMARK_TARGETS)
    add_dependencies(${target} fltx_source_identity)
    target_include_directories(${target} PRIVATE
        "${FLTX_METRICS_SOURCE_IDENTITY_DIR}"
    )
    target_compile_definitions(${target} PRIVATE
        FLTX_METRICS_EMBED_SOURCE_FINGERPRINT=1
    )
endforeach()

function(fltx_tests_add_native_accuracy_target
    target runner output_name label consumer_mode)
    add_custom_target(${target}
        COMMAND "${Python3_EXECUTABLE}"
            "${PROJECT_SOURCE_DIR}/validation/metrics/_internal/run_native_accuracy.py"
            --accuracy "$<TARGET_FILE:${runner}>"
            --output-root "${FLTX_VALIDATION_BINARY_DIR}/${output_name}"
            --sample-mode full
            --consumer-mode "${consumer_mode}"
        DEPENDS ${runner}
        USES_TERMINAL
        COMMENT "Running complete ${label} f32/f64 accuracy domains"
        VERBATIM
    )
endfunction()

fltx_tests_add_native_accuracy_target(
    fltx_native_accuracy_full
    fltx_accuracy
    native_accuracy
    strict-consumer
    strict
)
fltx_tests_add_native_accuracy_target(
    fltx_native_accuracy_fastmath_full
    fltx_accuracy_fastmath
    native_accuracy_fastmath
    fast-math-consumer
    fastmath
)

if(BUILD_TESTING)
    function(fltx_tests_add_accuracy_smoke
        target test_name output_name precision labels)
        set(output
            "${FLTX_VALIDATION_BINARY_DIR}/smoke/${output_name}_${precision}.csv"
        )
        add_test(
            NAME "${test_name}.${precision}.smoke"
            COMMAND ${target}
                --precision "${precision}"
                --output "${output}"
                --run-id "ctest-smoke"
                --source-revision "ctest"
                --sample-mode "smoke"
                ${ARGN}
        )
        set_tests_properties("${test_name}.${precision}.smoke" PROPERTIES
            LABELS "${labels}"
        )
    endfunction()

    foreach(precision f32 f64 f128 f256)
        set(FLTX_SIMULATED_FASTMATH_SMOKE_POLICY)
        if(precision STREQUAL "f32" OR precision STREQUAL "f64")
            set(FLTX_SIMULATED_FASTMATH_SMOKE_POLICY --advisory)
        endif()

        fltx_tests_add_accuracy_smoke(
            fltx_accuracy
            fltx.accuracy
            accuracy
            "${precision}"
            "fltx;accuracy;smoke"
        )
        fltx_tests_add_accuracy_smoke(
            fltx_accuracy_fastmath
            fltx.accuracy.fastmath
            accuracy_fastmath
            "${precision}"
            "fltx;accuracy;fastmath;smoke"
        )
        fltx_tests_add_accuracy_smoke(
            fltx_constexpr_accuracy
            fltx.constexpr_accuracy
            constexpr_accuracy
            "${precision}"
            "fltx;constexpr;accuracy;smoke"
        )
        fltx_tests_add_accuracy_smoke(
            fltx_constexpr_accuracy_fastmath
            fltx.constexpr_accuracy.fastmath
            constexpr_accuracy_fastmath
            "${precision}"
            "fltx;constexpr;accuracy;fastmath;smoke"
            ${FLTX_SIMULATED_FASTMATH_SMOKE_POLICY}
        )
    endforeach()

    function(fltx_tests_add_benchmark_smoke
        target test_name output_name precision labels)
        set(output
            "${FLTX_VALIDATION_BINARY_DIR}/smoke/${output_name}_${precision}.csv"
        )
        add_test(
            NAME "${test_name}.${precision}.smoke"
            COMMAND ${target}
                --precision "${precision}"
                --output "${output}"
                --run-id "ctest-smoke"
                --source-revision "ctest"
                --sample-mode "smoke"
        )
        set_tests_properties("${test_name}.${precision}.smoke" PROPERTIES
            LABELS "${labels}"
        )
    endfunction()

    foreach(precision f128 f256)
        fltx_tests_add_benchmark_smoke(
            fltx_benchmark
            fltx.benchmark
            benchmark
            "${precision}"
            "fltx;benchmark;smoke"
        )
        fltx_tests_add_benchmark_smoke(
            fltx_benchmark_fastmath
            fltx.benchmark.fastmath
            benchmark_fastmath
            "${precision}"
            "fltx;benchmark;fastmath;smoke"
        )
    endforeach()

    add_test(
        NAME fltx.metrics.tools
        COMMAND "${Python3_EXECUTABLE}"
            "${PROJECT_SOURCE_DIR}/validation/metrics/_internal/test_tools.py"
    )
    set_tests_properties(fltx.metrics.tools PROPERTIES LABELS "fltx;metrics")
endif()
