#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "fltx/detail/build_info.h"
#include "fltx/f256.h"

#ifndef FLTX_TEST_NAME
#define FLTX_TEST_NAME "unknown_tests"
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define FLTX_ARCH_ARM64_STATUS "defined"
#else
#define FLTX_ARCH_ARM64_STATUS "not defined"
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#define FLTX_ARCH_NEON_STATUS "defined"
#else
#define FLTX_ARCH_NEON_STATUS "not defined"
#endif

#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 2))
#define FLTX_ARCH_SSE2_STATUS "defined"
#else
#define FLTX_ARCH_SSE2_STATUS "not defined"
#endif

#if defined(__wasm_simd128__)
#define FLTX_ARCH_WASM_SIMD128_STATUS "defined"
#else
#define FLTX_ARCH_WASM_SIMD128_STATUS "not defined"
#endif

namespace
{
    [[nodiscard]] bool env_requests_simulated_consteval(bool fallback) noexcept
    {
        const char* value = std::getenv("FLTX_SIMULATE_CONSTEVAL");
        if (value == nullptr || *value == '\0')
            return fallback;

        if (std::strcmp(value, "0") == 0 ||
            std::strcmp(value, "false") == 0 ||
            std::strcmp(value, "FALSE") == 0 ||
            std::strcmp(value, "off") == 0 ||
            std::strcmp(value, "OFF") == 0)
        {
            return false;
        }

        return true;
    }

    [[nodiscard]] bool env_requests_macro_status_suppression() noexcept
    {
        const char* value = std::getenv("FLTX_TEST_SUPPRESS_MACRO_STATUS");
        if (value == nullptr || *value == '\0')
            return false;

        if (std::strcmp(value, "0") == 0 ||
            std::strcmp(value, "false") == 0 ||
            std::strcmp(value, "FALSE") == 0 ||
            std::strcmp(value, "off") == 0 ||
            std::strcmp(value, "OFF") == 0)
        {
            return false;
        }

        return true;
    }

    void print_numeric_state(const char* name, bool enabled) noexcept
    {
        std::fputs(name, stderr);
        std::fputs(enabled ? "1" : "0", stderr);
        std::fputc('\n', stderr);
    }

    void print_defined_state(const char* name, bool defined) noexcept
    {
        std::fputs(name, stderr);
        std::fputs(defined ? "defined" : "not defined", stderr);
        std::fputc('\n', stderr);
    }

    [[nodiscard]] const char* internal_fast_math_request_name(
        bl::detail::internal_fast_math_request request) noexcept
    {
        switch (request)
        {
        case bl::detail::internal_fast_math_request::automatic:
            return "AUTO";
        case bl::detail::internal_fast_math_request::enabled:
            return "ON";
        case bl::detail::internal_fast_math_request::disabled:
            return "OFF";
        }
        return "invalid";
    }

    [[nodiscard]] const char* configured_fma_mode_name(
        bl::detail::configured_fma_mode mode) noexcept
    {
        switch (mode)
        {
        case bl::detail::configured_fma_mode::automatic:
            return "AUTO";
        case bl::detail::configured_fma_mode::off:
            return "OFF";
        case bl::detail::configured_fma_mode::assume:
            return "ASSUME";
        }
        return "invalid";
    }

    void print_consumer_translation_unit() noexcept
    {
        std::fputs("[consumer translation unit]\nFLTX_FMA_MODE = ", stderr);
        #if FLTX_HEADER_FMA_AUTO
        std::fputs("AUTO\n", stderr);
        #elif FLTX_HEADER_FMA_OFF
        std::fputs("OFF\n", stderr);
        #elif FLTX_HEADER_FMA_ASSUME
        std::fputs("ASSUME\n", stderr);
        #else
        std::fputs("invalid\n", stderr);
        #endif

        std::fputs("FLTX_SIMD = ", stderr);
        #if FLTX_HEADER_SIMD_OFF
        std::fputs("OFF\n", stderr);
        #else
        std::fputs("ON\n", stderr);
        #endif

        #if defined(FLTX_FAST_MATH)
        print_defined_state("FLTX_FAST_MATH = ", true);
        #else
        print_defined_state("FLTX_FAST_MATH = ", false);
        #endif

        print_numeric_state("FLTX_X86_TARGET = ", FLTX_X86_TARGET);
        print_numeric_state("FLTX_ARM64_TARGET = ", FLTX_ARM64_TARGET);
        print_numeric_state("FLTX_TU_HAS_X86_FMA = ", FLTX_TU_HAS_X86_FMA);
        print_numeric_state("FLTX_HAS_X86_FMA = ", FLTX_HAS_X86_FMA);
        print_numeric_state(
            "FLTX_HAS_COMPILED_X86_FMA_BACKEND = ",
            FLTX_HAS_COMPILED_X86_FMA_BACKEND);
        print_numeric_state(
            "FLTX_DETAIL_X86_FMA_RUNTIME_CHECK = ",
            FLTX_DETAIL_X86_FMA_RUNTIME_CHECK);
        print_numeric_state(
            "FLTX_DETAIL_MSVC_GUARDED_X86_FMA = ",
            FLTX_DETAIL_MSVC_GUARDED_X86_FMA);
        print_numeric_state(
            "FLTX_DETAIL_USE_SCALAR_X86_FMA = ",
            FLTX_DETAIL_USE_SCALAR_X86_FMA);
        print_numeric_state(
            "FLTX_DETAIL_USE_BASELINE_ARM64_FMA = ",
            FLTX_DETAIL_USE_BASELINE_ARM64_FMA);
        print_numeric_state(
            "FLTX_DETAIL_HAS_RUNTIME_FMA_PATH = ",
            FLTX_DETAIL_HAS_RUNTIME_FMA_PATH);

        std::fputs("runtime_fma = ", stderr);
        #if FLTX_DETAIL_X86_FMA_RUNTIME_CHECK
        std::fputs(
            bl::detail::fp::runtime_hardware_fma_enabled() ? "detected\n" : "unavailable\n",
            stderr);
        #elif FLTX_HEADER_FMA_ASSUME
        std::fputs("assumed\n", stderr);
        #elif FLTX_DETAIL_USE_SCALAR_X86_FMA
        std::fputs("compile-time\n", stderr);
        #elif FLTX_DETAIL_USE_BASELINE_ARM64_FMA
        std::fputs("baseline\n", stderr);
        #else
        std::fputs("off\n", stderr);
        #endif

        print_numeric_state("FLTX_HAS_SSE2 = ", FLTX_HAS_SSE2);
        print_numeric_state("FLTX_HAS_NEON = ", FLTX_HAS_NEON);
        print_numeric_state("FLTX_HAS_WASM_SIMD = ", FLTX_HAS_WASM_SIMD);
        print_numeric_state("FLTX_F256_ENABLE_SIMD = ", FLTX_F256_ENABLE_SIMD);
        print_numeric_state("FLTX_F256_ENABLE_TRIG_SIMD = ", FLTX_F256_ENABLE_TRIG_SIMD);
        print_numeric_state("FLTX_SIMD_USE_FMA_TWO_PROD = ", FLTX_SIMD_USE_FMA_TWO_PROD);

        print_defined_state("compiler_arm64 = ", std::strcmp(FLTX_ARCH_ARM64_STATUS, "defined") == 0);
        print_defined_state("compiler_neon = ", std::strcmp(FLTX_ARCH_NEON_STATUS, "defined") == 0);
        print_defined_state("compiler_sse2 = ", std::strcmp(FLTX_ARCH_SSE2_STATUS, "defined") == 0);
        print_defined_state(
            "compiler_wasm_simd128 = ",
            std::strcmp(FLTX_ARCH_WASM_SIMD128_STATUS, "defined") == 0);
    }

    void print_library_build() noexcept
    {
        const bl::detail::compiled_build_info& info = bl::detail::library_build_info();

        std::fputs("\n[fltx compiled library]\nFLTX_FMA_MODE = ", stderr);
        std::fputs(configured_fma_mode_name(info.fma_mode), stderr);
        std::fputs("\nFLTX_INTERNAL_FAST_MATH = ", stderr);
        std::fputs(internal_fast_math_request_name(info.fast_math_request), stderr);
        std::fputc('\n', stderr);
        print_defined_state("FLTX_FAST_MATH = ", info.fast_math_enabled);
        std::fputs("FLTX_SIMD = ", stderr);
        std::fputs(info.simd_policy_enabled ? "ON\n" : "OFF\n", stderr);

        print_numeric_state("FLTX_X86_TARGET = ", info.x86_target);
        print_numeric_state("FLTX_ARM64_TARGET = ", info.arm64_target);
        print_numeric_state("FLTX_TU_HAS_X86_FMA = ", info.tu_has_x86_fma);
        print_numeric_state("FLTX_HAS_X86_FMA = ", info.has_x86_fma);
        print_numeric_state(
            "FLTX_HAS_COMPILED_X86_FMA_BACKEND = ",
            info.compiled_x86_fma_backend);
        print_numeric_state(
            "FLTX_DETAIL_X86_FMA_RUNTIME_CHECK = ",
            info.x86_fma_runtime_check);
        print_numeric_state(
            "FLTX_DETAIL_MSVC_GUARDED_X86_FMA = ",
            info.msvc_guarded_x86_fma);
        print_numeric_state("FLTX_DETAIL_USE_SCALAR_X86_FMA = ", info.scalar_x86_fma);
        print_numeric_state(
            "FLTX_DETAIL_USE_BASELINE_ARM64_FMA = ",
            info.baseline_arm64_fma);
        print_numeric_state(
            "FLTX_DETAIL_HAS_RUNTIME_FMA_PATH = ",
            info.has_runtime_fma_path);

        print_numeric_state("FLTX_HAS_SSE2 = ", info.has_sse2);
        print_numeric_state("FLTX_HAS_NEON = ", info.has_neon);
        print_numeric_state("FLTX_HAS_WASM_SIMD = ", info.has_wasm_simd);
        print_numeric_state("FLTX_F256_ENABLE_SIMD = ", info.f256_simd_enabled);
        print_numeric_state(
            "FLTX_F256_ENABLE_TRIG_SIMD = ",
            info.f256_trig_simd_enabled);
        print_numeric_state(
            "FLTX_SIMD_USE_FMA_TWO_PROD = ",
            info.simd_fma_two_prod);

        std::fputs("\n[fltx hardware FMA path]\nstrategy = ", stderr);
        if (info.compiled_x86_fma_backend)
            std::fputs("whole-operation x86 backend\n", stderr);
        else if (info.msvc_guarded_x86_fma)
            std::fputs("MSVC guarded inline intrinsics\n", stderr);
        else if (info.scalar_x86_fma)
            std::fputs("direct translation-unit x86 FMA\n", stderr);
        else if (info.baseline_arm64_fma)
            std::fputs("baseline AArch64 FMA\n", stderr);
        else
            std::fputs("disabled (inline Dekker)\n", stderr);

        if (info.compiled_x86_fma_backend)
            print_numeric_state("specialized_backend_tu_has_x86_fma = ", true);

        std::fputs("runtime_fma = ", stderr);
        if (info.x86_fma_runtime_check)
        {
            std::fputs(
                bl::detail::library_runtime_hardware_fma_enabled()
                    ? "detected\n"
                    : "unavailable\n",
                stderr);
        }
        else if (info.scalar_x86_fma)
        {
            std::fputs("compile-time\n", stderr);
        }
        else if (info.baseline_arm64_fma)
        {
            std::fputs("baseline\n", stderr);
        }
        else
        {
            std::fputs("off\n", stderr);
        }
    }

    struct fltx_test_macro_status_printer
    {
        fltx_test_macro_status_printer() noexcept
        {
            bool simulated_consteval_enabled = false;
            #if defined(FLTX_SIMULATE_TOGGLE_CONSTEVAL_MODE)
            #if defined(FLTX_TEST_FORCE_SIMULATED_CONSTEVAL)
            simulated_consteval_enabled = true;
            #endif
            simulated_consteval_enabled = env_requests_simulated_consteval(simulated_consteval_enabled);
            bl::_fltx_debug::set_simulated_consteval_path(simulated_consteval_enabled);
            #endif

            if (env_requests_macro_status_suppression())
                return;

            std::fputs("[fltx ", stderr);
            std::fputs(FLTX_TEST_NAME, stderr);
            std::fputs("]\n\n", stderr);

            print_consumer_translation_unit();
            print_library_build();

            std::fputs("\n[test execution]\n", stderr);
            #if defined(FLTX_CONSTEXPR_PARITY)
            print_defined_state("FLTX_CONSTEXPR_PARITY = ", true);
            #else
            print_defined_state("FLTX_CONSTEXPR_PARITY = ", false);
            #endif

            #if defined(FLTX_SIMULATE_TOGGLE_CONSTEVAL_MODE)
            print_defined_state("FLTX_SIMULATE_TOGGLE_CONSTEVAL_MODE = ", true);
            #else
            print_defined_state("FLTX_SIMULATE_TOGGLE_CONSTEVAL_MODE = ", false);
            #endif

            #if defined(FLTX_SIMULATE_FIXED_CONSTEVAL_MODE)
            print_defined_state("FLTX_SIMULATE_FIXED_CONSTEVAL_MODE = ", true);
            #else
            print_defined_state("FLTX_SIMULATE_FIXED_CONSTEVAL_MODE = ", false);
            #endif

            #if defined(FLTX_TEST_FORCE_SIMULATED_CONSTEVAL)
            print_defined_state("FLTX_TEST_FORCE_SIMULATED_CONSTEVAL = ", true);
            #else
            print_defined_state("FLTX_TEST_FORCE_SIMULATED_CONSTEVAL = ", false);
            #endif

            #if defined(FLTX_METRICS_BENCHMARK_ONLY_FLTX)
            print_defined_state("FLTX_METRICS_BENCHMARK_ONLY_FLTX = ", true);
            #else
            print_defined_state("FLTX_METRICS_BENCHMARK_ONLY_FLTX = ", false);
            #endif

            std::fputs("simulated_consteval = ", stderr);
            #if defined(FLTX_SIMULATE_FIXED_CONSTEVAL_MODE)
            std::fputs("fixed\n", stderr);
            #elif defined(FLTX_SIMULATE_TOGGLE_CONSTEVAL_MODE)
            std::fputs(simulated_consteval_enabled ? "on\n" : "off\n", stderr);
            #else
            std::fputs("unavailable\n", stderr);
            #endif

            std::fputc('\n', stderr);
            std::fflush(stderr);
        }
    };

    const fltx_test_macro_status_printer fltx_test_macro_status_printer_instance{};

} // namespace
