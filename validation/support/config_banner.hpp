#pragma once

#include "source_identity.hpp"

#include <cstddef>
#include <cstdio>

#include <fltx/config.h>
#include <fltx/detail/build_info.h>

#ifndef FLTX_TESTS_COMPILER_ID
#define FLTX_TESTS_COMPILER_ID "unknown"
#endif

#ifndef FLTX_TESTS_COMPILER_VERSION
#define FLTX_TESTS_COMPILER_VERSION "unknown"
#endif

#ifndef FLTX_TESTS_BUILD_CONFIG
#define FLTX_TESTS_BUILD_CONFIG "unknown"
#endif

#ifndef FLTX_TESTS_SYSTEM_NAME
#define FLTX_TESTS_SYSTEM_NAME "unknown"
#endif

#ifndef FLTX_TESTS_SYSTEM_PROCESSOR
#define FLTX_TESTS_SYSTEM_PROCESSOR "unknown"
#endif

#ifndef FLTX_TESTS_BUILD_OPTIMIZED
#define FLTX_TESTS_BUILD_OPTIMIZED 0
#endif

#ifndef FLTX_TESTS_BUILD_FLAGS_HASH
#define FLTX_TESTS_BUILD_FLAGS_HASH "unknown"
#endif

#ifndef FLTX_METRICS_QDPP_ENABLED
#define FLTX_METRICS_QDPP_ENABLED 0
#endif

#ifndef FLTX_METRICS_HAS_TLFLOAT
#define FLTX_METRICS_HAS_TLFLOAT 0
#endif

#if defined(FLTX_TESTS_EXPECT_CONSUMER_FAST_MATH)
  #if FLTX_TESTS_EXPECT_CONSUMER_FAST_MATH && !defined(FLTX_FAST_MATH)
    #error "consumer fast-math validation target was not compiled in fast-math mode"
  #elif !FLTX_TESTS_EXPECT_CONSUMER_FAST_MATH && defined(FLTX_FAST_MATH)
    #error "strict consumer validation target was compiled in fast-math mode"
  #endif
#endif

namespace fltx::tests::support
{
    inline const char* fma_mode_name(bl::detail::configured_fma_mode mode) noexcept
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

    inline const char* fast_math_request_name(
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

    inline void print_config_banner(const char* test_name, const char* sample_mode = nullptr,
                                    std::size_t samples = 0, std::size_t trials = 0) noexcept
    {
        const auto& library = bl::detail::library_build_info();

        const char* fma = "AUTO";
        if constexpr (FLTX_HEADER_FMA_OFF)
            fma = "OFF";
        else if constexpr (FLTX_HEADER_FMA_ASSUME)
            fma = "ASSUME";

        std::fprintf(stderr, "[fltx %s]\n", test_name);
        if (sample_mode != nullptr)
        {
            std::fprintf(stderr, "[run] sample-mode=%s samples=%zu trials=%zu\n", sample_mode,
                         samples, trials);
        }
        std::fprintf(stderr, "[harness] qdpp=%s tlfloat=%s\n", FLTX_METRICS_QDPP_ENABLED ? "on" : "off",
                     FLTX_METRICS_HAS_TLFLOAT ? "on" : "off");
        std::fprintf(stderr,
                     "[implementations] f128=fltx%s,cppdd%s "
                     "f256=fltx%s,mpfr64%s\n",
                     FLTX_METRICS_QDPP_ENABLED ? ",qdpp" : "", FLTX_METRICS_HAS_TLFLOAT ? ",tlfloat" : "",
                     FLTX_METRICS_QDPP_ENABLED ? ",qdpp" : "", FLTX_METRICS_HAS_TLFLOAT ? ",tlfloat" : "");
        std::fprintf(stderr,
                     "[build] compiler-id=%s compiler-version=%s config=%s optimized=%d "
                     "system=%s processor=%s flags-hash=%s source-fingerprint=%s\n",
                     FLTX_TESTS_COMPILER_ID, FLTX_TESTS_COMPILER_VERSION, FLTX_TESTS_BUILD_CONFIG,
                     FLTX_TESTS_BUILD_OPTIMIZED, FLTX_TESTS_SYSTEM_NAME, FLTX_TESTS_SYSTEM_PROCESSOR,
                     FLTX_TESTS_BUILD_FLAGS_HASH, source_fingerprint);
        std::fprintf(stderr,
                     "[consumer] fma=%s simd=%s fast-math=%s simulated-consteval=%s "
                     "x86=%d arm64=%d tu-x86-fma=%d has-x86-fma=%d\n",
                     fma, FLTX_HEADER_SIMD_OFF ? "off" : "on",
#if defined(FLTX_FAST_MATH)
                     "on",
#else
                     "off",
#endif
#if defined(FLTX_SIMULATE_FIXED_CONSTEVAL_MODE)
                     "on",
#else
                     "off",
#endif
                     FLTX_X86_TARGET, FLTX_ARM64_TARGET, FLTX_TU_HAS_X86_FMA, FLTX_HAS_X86_FMA);

        std::fprintf(stderr,
                     "[fltx lib] fma=%s simd=%s fast-math-request=%s fast-math=%s "
                     "x86=%d arm64=%d tu-x86-fma=%d runtime-fma=%s\n",
                     fma_mode_name(library.fma_mode), library.simd_policy_enabled ? "on" : "off",
                     fast_math_request_name(library.fast_math_request),
                     library.fast_math_enabled ? "on" : "off", library.x86_target,
                     library.arm64_target, library.tu_has_x86_fma,
                     bl::detail::library_runtime_hardware_fma_enabled() ? "available"
                                                                        : "unavailable");

        std::fprintf(stderr,
                     "[fltx lib strategy] compiled-x86-backend=%d runtime-check=%d "
                     "msvc-guarded=%d scalar-x86=%d baseline-arm64=%d runtime-path=%d "
                     "sse2=%d neon=%d wasm-simd=%d f256-simd=%d trig-simd=%d\n",
                     library.compiled_x86_fma_backend, library.x86_fma_runtime_check,
                     library.msvc_guarded_x86_fma, library.scalar_x86_fma,
                     library.baseline_arm64_fma, library.has_runtime_fma_path, library.has_sse2,
                     library.has_neon, library.has_wasm_simd, library.f256_simd_enabled,
                     library.f256_trig_simd_enabled);
    }
} // namespace fltx::tests::support
