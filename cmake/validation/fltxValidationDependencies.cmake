# Validation-only numerical oracles and comparison libraries.

find_path(FLTX_TESTS_MPFR_INCLUDE_DIR NAMES mpfr.h REQUIRED)
find_library(FLTX_TESTS_MPFR_LIBRARY NAMES mpfr REQUIRED)
find_path(FLTX_TESTS_GMP_INCLUDE_DIR NAMES gmp.h REQUIRED)
find_library(FLTX_TESTS_GMP_LIBRARY NAMES gmp REQUIRED)

option(FLTX_METRICS_EXTERNAL_COMPARISONS
    "Build external implementations used only by metrics comparisons."
    ON
)

option(FLTX_METRICS_TLFLOAT
    "Build the vendored TLFloat dependency for metrics comparisons."
    ON
)

set(FLTX_METRICS_TLFLOAT_CONTRACT_TARGET)
if(FLTX_METRICS_EXTERNAL_COMPARISONS AND FLTX_METRICS_TLFLOAT)
    set(FLTX_METRICS_TLFLOAT_SOURCE_DIR
        "${FLTX_VALIDATION_SOURCE_DIR}/extern/tlfloat"
    )
    set(FLTX_METRICS_TLFLOAT_BINARY_DIR
        "${FLTX_VALIDATION_BINARY_DIR}/extern/tlfloat"
    )

    if(NOT EXISTS "${FLTX_METRICS_TLFLOAT_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "FLTX_METRICS_TLFLOAT is ON, but validation/extern/tlfloat is unavailable."
        )
    endif()

    # TLFloat's standalone project uses intentionally broad option names.
    # Keep them in this function scope so they cannot become fltx options or
    # alter the parent build. Only its static library is needed here.
    function(fltx_tests_add_tlfloat)
        set(BUILD_LIBS ON)
        set(BUILD_TESTS OFF)
        set(BUILD_UTILS OFF)
        set(BUILD_BENCH OFF)
        set(BUILD_SHARED_LIBS OFF)
        set(ENABLE_DOXYGEN OFF)
        set(ENABLE_COVERAGE OFF)
        set(BUILD_EXHAUSTIVE_TESTING OFF)
        set(ENABLE_EXHAUSTIVE_TESTING OFF)
        add_subdirectory(
            "${FLTX_METRICS_TLFLOAT_SOURCE_DIR}"
            "${FLTX_METRICS_TLFLOAT_BINARY_DIR}"
            EXCLUDE_FROM_ALL
        )
    endfunction()
    fltx_tests_add_tlfloat()

    set_target_properties(tlfloat PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${FLTX_METRICS_TLFLOAT_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${FLTX_METRICS_TLFLOAT_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${FLTX_METRICS_TLFLOAT_BINARY_DIR}/bin"
    )
    set_target_properties(tlfloat_inline PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${FLTX_METRICS_TLFLOAT_BINARY_DIR}/lib"
    )

    add_library(fltx_tlfloat_dependency INTERFACE)
    target_link_libraries(fltx_tlfloat_dependency INTERFACE tlfloat)
    target_include_directories(fltx_tlfloat_dependency INTERFACE
        "${FLTX_METRICS_TLFLOAT_SOURCE_DIR}/src/include"
        "${FLTX_METRICS_TLFLOAT_BINARY_DIR}/include"
    )
    add_library(fltx::test_tlfloat ALIAS fltx_tlfloat_dependency)

    add_executable(fltx_tlfloat_smoke
        "${FLTX_VALIDATION_SOURCE_DIR}/support/tlfloat_smoke.cpp"
    )
    target_link_libraries(fltx_tlfloat_smoke PRIVATE
        fltx::test_tlfloat
    )
    target_compile_features(fltx_tlfloat_smoke PRIVATE cxx_std_20)
    set(FLTX_METRICS_TLFLOAT_CONTRACT_TARGET fltx_tlfloat_smoke)

    if(BUILD_TESTING)
        add_test(NAME fltx.tlfloat.link COMMAND fltx_tlfloat_smoke)
        set_tests_properties(fltx.tlfloat.link PROPERTIES
            LABELS "fltx;metrics;tlfloat"
        )
    endif()

    message(STATUS
        "fltx metrics: TLFloat dependency enabled from validation/extern/tlfloat."
    )
endif()
