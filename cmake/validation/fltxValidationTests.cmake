# Contract, constexpr, and compile-surface validation targets.

set(FLTX_CONTRACT_TEST_SOURCE_FILE_PATHS
    "${FLTX_VALIDATION_SOURCE_DIR}/support/catch_main.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/contracts/core.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/contracts/math.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/contracts/io.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/contracts/random.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/contracts/expressions.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/contracts/dispatch.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/contracts/edge_cases.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/contracts/overloads.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/contracts/overload_matrix.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/contracts/approx_comparison.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/contracts/decimal.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/contracts/benchmark_policy.cpp"
)

function(fltx_tests_add_contract_runner target label consumer_fast_math)
    add_executable(${target} ${FLTX_CONTRACT_TEST_SOURCE_FILE_PATHS})
    fltx_tests_prepare_test(${target} "${label}" fltx::fltx)
    fltx_tests_configure_consumer_mode(${target} "${consumer_fast_math}")
endfunction()

fltx_tests_add_contract_runner(fltx_contract_tests contracts OFF)
fltx_tests_add_contract_runner(
    fltx_contract_tests_fastmath
    contracts-fastmath
    ON
)

# Compile an isolated fixed-consteval library so the complete accuracy runner can
# exercise constexpr algorithms at runtime without changing normal applications.
add_library(fltx_constexpr_accuracy_lib STATIC ${FLTX_LIBRARY_SOURCE_FILE_PATHS})
target_include_directories(fltx_constexpr_accuracy_lib PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)
target_compile_definitions(fltx_constexpr_accuracy_lib PUBLIC
    FLTX_SIMULATE_FIXED_CONSTEVAL_MODE
)
fltx_configure_library_target(fltx_constexpr_accuracy_lib)
fltx_enable_msvc_parallel_compile(fltx_constexpr_accuracy_lib)

# GCC defaults to permitting contraction even in otherwise strict translation
# units, and its STDC FP_CONTRACT pragma does not reliably protect the inline
# error-free transforms on AArch64. Fixed simulation must retain the separate
# rounding steps that genuine constant evaluation observes.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(fltx_constexpr_accuracy_lib PRIVATE
        -ffp-contract=off
    )
endif()

set(FLTX_CONSTEXPR_TEST_SOURCE_FILE_PATHS
    "${FLTX_VALIDATION_SOURCE_DIR}/support/catch_main.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/constexpr/core.cpp"
    "${FLTX_VALIDATION_SOURCE_DIR}/tests/constexpr/accuracy.cpp"
)

# The genuine-constant-evaluation corpus stores compiler-evaluated results and
# checks them against the same high-precision oracle used by the accuracy runner.
function(fltx_tests_add_constexpr_runner target label consumer_fast_math)
    add_executable(${target} ${FLTX_CONSTEXPR_TEST_SOURCE_FILE_PATHS})
    fltx_tests_prepare_test(${target} "${label}" fltx::fltx)
    fltx_tests_configure_consumer_mode(${target} "${consumer_fast_math}")

    target_include_directories(${target} PRIVATE
        "${FLTX_TESTS_MPFR_INCLUDE_DIR}"
        "${FLTX_TESTS_GMP_INCLUDE_DIR}"
    )
    target_link_libraries(${target} PRIVATE
        "${FLTX_TESTS_MPFR_LIBRARY}"
        "${FLTX_TESTS_GMP_LIBRARY}"
    )
endfunction()

fltx_tests_add_constexpr_runner(fltx_constexpr_tests constexpr OFF)
fltx_tests_add_constexpr_runner(
    fltx_constexpr_tests_fastmath
    constexpr-fastmath
    ON
)

# Every public header must compile by itself. Sources are generated in the build
# tree so the source tree stays small and the probes always follow the headers.
file(GLOB FLTX_TESTS_PUBLIC_HEADERS CONFIGURE_DEPENDS
    RELATIVE "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/include/fltx/*.h"
    "${PROJECT_SOURCE_DIR}/include/fltx/util/*.h"
)

set(FLTX_TESTS_HEADER_PROBE_SOURCES)
foreach(header IN LISTS FLTX_TESTS_PUBLIC_HEADERS)
    string(MAKE_C_IDENTIFIER "${header}" probe_name)
    set(source "${FLTX_VALIDATION_BINARY_DIR}/header_probes/${probe_name}.cpp")
    if(header STREQUAL "fltx/native.h" OR
       header STREQUAL "fltx/native_math.h" OR
       header STREQUAL "fltx/native_string.h")
        string(CONCAT probe_content
            "#include <${header}>\n\n"
            "template<class T>\n"
            "concept fltx_complete_type = requires { sizeof(T); };\n\n"
            "static_assert(!fltx_complete_type<bl::fdd>);\n"
            "static_assert(!fltx_complete_type<bl::fdd_s>);\n"
            "static_assert(!fltx_complete_type<bl::fqd>);\n"
            "static_assert(!fltx_complete_type<bl::fqd_s>);\n"
        )
        file(GENERATE OUTPUT "${source}" CONTENT "${probe_content}")
    else()
        file(GENERATE OUTPUT "${source}" CONTENT "#include <${header}>\n")
    endif()
    list(APPEND FLTX_TESTS_HEADER_PROBE_SOURCES "${source}")
endforeach()

add_library(fltx_header_contract OBJECT EXCLUDE_FROM_ALL
    ${FLTX_TESTS_HEADER_PROBE_SOURCES}
)
target_link_libraries(fltx_header_contract PRIVATE fltx::fltx)
target_compile_features(fltx_header_contract PRIVATE cxx_std_20)

function(fltx_tests_add_cxx_standard_contract target standard)
    add_library(${target} OBJECT EXCLUDE_FROM_ALL
        "${FLTX_VALIDATION_SOURCE_DIR}/tests/standard/cxx_standard_contract.cpp"
    )
    target_link_libraries(${target} PRIVATE fltx::fltx)
    target_compile_definitions(${target} PRIVATE
        "FLTX_CXX_STANDARD_CONTRACT=${standard}"
    )
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD ${standard}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    if(MSVC)
        target_compile_options(${target} PRIVATE /permissive- /Zc:__cplusplus)
    else()
        target_compile_options(${target} PRIVATE -pedantic-errors)
    endif()
endfunction()

fltx_tests_add_cxx_standard_contract(fltx_cxx20_contract 20)
fltx_tests_add_cxx_standard_contract(fltx_cxx23_contract 23)
