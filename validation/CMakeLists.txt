find_package(Catch2 3 CONFIG REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
include(Catch)

set(FLTX_VALIDATION_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(FLTX_VALIDATION_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(FLTX_TESTS_SUPPORT_DIR "${FLTX_VALIDATION_SOURCE_DIR}/support")

include("${PROJECT_SOURCE_DIR}/cmake/validation/fltxValidationCommon.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/validation/fltxValidationDependencies.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/validation/fltxValidationTests.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/validation/fltxValidationPackage.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/validation/fltxValidationRunners.cmake")

add_custom_target(fltx_compile_contract
    DEPENDS
        fltx_header_contract
        fltx_cxx20_contract
        fltx_cxx23_contract
        fltx_contract_tests
        fltx_contract_tests_fastmath
        fltx_constexpr_tests
        fltx_constexpr_tests_fastmath
        fltx_package_smoke
        ${FLTX_METRICS_TLFLOAT_CONTRACT_TARGET}
        ${FLTX_METRICS_QDPP_CONTRACT_TARGET}
)

if(BUILD_TESTING)
    set(FLTX_CI_RELEASE_ACCURACY_DIR
        "${FLTX_VALIDATION_BINARY_DIR}/release_accuracy"
    )
    set(FLTX_CI_RELEASE_ACCURACY_COMMANDS)
    foreach(precision f32 f64 f128 f256)
        list(APPEND FLTX_CI_RELEASE_ACCURACY_COMMANDS
            COMMAND fltx_accuracy
                --precision "${precision}"
                --output "${FLTX_CI_RELEASE_ACCURACY_DIR}/accuracy_${precision}.csv"
                --run-id ci-release
                --source-revision ci
                --sample-mode full
            COMMAND fltx_constexpr_accuracy
                --precision "${precision}"
                --output "${FLTX_CI_RELEASE_ACCURACY_DIR}/constexpr_accuracy_${precision}.csv"
                --run-id ci-release
                --source-revision ci
                --sample-mode full
            COMMAND fltx_accuracy_fastmath
                --precision "${precision}"
                --output "${FLTX_CI_RELEASE_ACCURACY_DIR}/accuracy_${precision}_fastmath.csv"
                --run-id ci-release
                --source-revision ci
                --sample-mode full
            COMMAND fltx_constexpr_accuracy_fastmath
                --precision "${precision}"
                --output "${FLTX_CI_RELEASE_ACCURACY_DIR}/constexpr_accuracy_${precision}_fastmath.csv"
                --run-id ci-release
                --source-revision ci
                --sample-mode full
        )
    endforeach()

    # Badge workflows build this target. Contracts and tooling run through
    # CTest, while accuracy uses the complete release corpus for every type and
    # both runtime/constexpr-algorithm libraries. Performance telemetry and
    # canonical metrics publication remain separate release-evidence tasks.
    add_custom_target(fltx_ci_checks
        COMMAND "${CMAKE_COMMAND}" -E make_directory
            "${FLTX_CI_RELEASE_ACCURACY_DIR}"
        COMMAND "${CMAKE_CTEST_COMMAND}"
            --test-dir "${CMAKE_BINARY_DIR}"
            --build-config "$<CONFIG>"
            --output-on-failure
            -L "contracts|constexpr|metrics|package"
            -LE "accuracy|benchmark"
        ${FLTX_CI_RELEASE_ACCURACY_COMMANDS}
        DEPENDS
            fltx_compile_contract
            fltx_accuracy
            fltx_constexpr_accuracy
            fltx_accuracy_fastmath
            fltx_constexpr_accuracy_fastmath
        USES_TERMINAL
        COMMENT "Running fltx contracts and release-grade accuracy corpus"
        VERBATIM
    )
endif()

add_subdirectory(benchmarks)
