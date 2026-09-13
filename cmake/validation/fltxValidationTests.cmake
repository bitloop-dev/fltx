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
    set(probe_content "#include <${header}>\n")
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
    endif()

    set(probe_families)
    if(header MATCHES "^fltx/(fdd|fqd)(_type|_limits|_classification|_comparison|_arithmetic)?\\.h$")
        set(probe_families "${CMAKE_MATCH_1}")
    elseif(header STREQUAL "fltx/hash.h" OR header STREQUAL "fltx/limits.h")
        set(probe_families fdd fqd)
    endif()
    if(probe_families)
        string(APPEND probe_content [=[
#include <cstdint>
#include <limits>
template<class Value, class Storage>
constexpr bool fltx_header_integer_construction()
{
    constexpr auto u = std::numeric_limits<std::uint64_t>::max();
    constexpr auto s = std::numeric_limits<std::int64_t>::min();
    const Value small{1}, large{u}, negative{s};
    const Value signed_max{std::numeric_limits<std::int64_t>::max()};
    Storage assigned{};
    if constexpr (requires { assigned.x2; }) { assigned.x2 = 3.0; assigned.x3 = 4.0; }
    if (&(assigned = u) != &assigned) return false;
    if constexpr (requires { assigned.hi; })
        return small.hi == 1.0 && small.lo == 0.0 &&
               large.hi == 0x1p64 && large.lo == -1.0 &&
               negative.hi == -0x1p63 && negative.lo == 0.0 &&
               signed_max.hi == 0x1p63 && signed_max.lo == -1.0 &&
               assigned.hi == large.hi && assigned.lo == large.lo;
    else
        return small.x0 == 1.0 && small.x1 == 0.0 &&
               large.x0 == 0x1p64 && large.x1 == -1.0 &&
               negative.x0 == -0x1p63 && negative.x1 == 0.0 &&
               signed_max.x0 == 0x1p63 && signed_max.x1 == -1.0 &&
               assigned.x0 == large.x0 && assigned.x1 == large.x1 &&
               assigned.x2 == 0.0 && assigned.x3 == 0.0;
}
]=])
        foreach(family IN LISTS probe_families)
            string(APPEND probe_content
                "static_assert(fltx_header_integer_construction<bl::${family}, bl::${family}_s>());\n"
                "bl::${family} ${probe_name}_${family}_signed(std::int64_t v) { return bl::${family}{v}; }\n"
                "bl::${family} ${probe_name}_${family}_unsigned(std::uint64_t v) { return bl::${family}{v}; }\n"
            )
        endforeach()
    endif()
    if(header STREQUAL "fltx/fqd_arithmetic.h" OR header STREQUAL "fltx/fqd.h")
        string(APPEND probe_content [=[
template<class T> constexpr bool fltx_header_selected_qd_result =
#if defined(FLTX_ENABLE_FQD_EXPRESSIONS) && FLTX_ENABLE_FQD_EXPRESSIONS
    bl::detail::_qd_expr::is_expr<T>::value;
#else
    std::is_same_v<T, bl::fqd>;
#endif
static_assert(fltx_header_selected_qd_result<decltype(bl::fqd{2} + bl::fqd{3})>);
static_assert(fltx_header_selected_qd_result<decltype(bl::fqd{2} - bl::fqd{3})>);
static_assert(fltx_header_selected_qd_result<decltype(bl::fqd{2} * bl::fqd{3})>);
static_assert(fltx_header_selected_qd_result<decltype(bl::fqd{2} / bl::fqd{3})>);
constexpr bl::fqd fltx_header_expression = bl::fqd{2} * bl::fqd{3} + bl::fqd{1};
static_assert(fltx_header_expression.x0 == 7.0);
using fltx_header_sum_before_umbrella = decltype(bl::fqd{2} + bl::fqd{3});
#include <fltx/fqd.h>
#include <fltx/fqd_arithmetic.h>
static_assert(std::is_same_v<fltx_header_sum_before_umbrella, decltype(bl::fqd{2} + bl::fqd{3})>);
]=])
    endif()
    file(GENERATE OUTPUT "${source}" CONTENT "${probe_content}")
    list(APPEND FLTX_TESTS_HEADER_PROBE_SOURCES "${source}")
endforeach()

foreach(target fltx_header_contract fltx_expression_header_contract)
    add_library(${target} OBJECT EXCLUDE_FROM_ALL
        ${FLTX_TESTS_HEADER_PROBE_SOURCES}
    )
    target_link_libraries(${target} PRIVATE fltx::fltx)
    target_compile_features(${target} PRIVATE cxx_std_20)
endforeach()
target_compile_definitions(fltx_expression_header_contract PRIVATE
    FLTX_ENABLE_FQD_EXPRESSIONS=1
)

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

fltx_tests_add_cxx_standard_contract(fltx_math_dekker_disabled_contract 20)
target_compile_definitions(fltx_math_dekker_disabled_contract PRIVATE
    FLTX_DISABLE_MATH_USES_CHECKED_DEKKER
)

foreach(standard 20 23)
    fltx_tests_add_cxx_standard_contract(fltx_cxx${standard}_expressions_contract ${standard})
    target_compile_definitions(fltx_cxx${standard}_expressions_contract PRIVATE
        FLTX_ENABLE_FQD_EXPRESSIONS=1
    )
endforeach()
