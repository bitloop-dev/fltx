/**
 * fltx/src/build_info.cpp - Capture the baseline compiled-library configuration.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#include "fltx/detail/build_info.h"
#include "fltx/detail/common_fp.h"
#include "fltx/detail/f256_simd_config.h"

#if FLTX_DETAIL_X86_FMA_RUNTIME_CHECK
#  if defined(_MSC_VER)
#    include <intrin.h>
#  elif defined(__GNUC__) || defined(__clang__)
#    include <cpuid.h>
#  endif
#endif

namespace
{
    constexpr bl::detail::configured_fma_mode configured_fma_policy =
        #if FLTX_HEADER_FMA_OFF
        bl::detail::configured_fma_mode::off;
        #elif FLTX_HEADER_FMA_ASSUME
        bl::detail::configured_fma_mode::assume;
        #else
        bl::detail::configured_fma_mode::automatic;
        #endif

    constexpr bl::detail::internal_fast_math_request configured_fast_math_request =
        #if defined(FLTX_INTERNAL_FAST_MATH_REQUEST_ON)
        bl::detail::internal_fast_math_request::enabled;
        #elif defined(FLTX_INTERNAL_FAST_MATH_REQUEST_OFF)
        bl::detail::internal_fast_math_request::disabled;
        #else
        bl::detail::internal_fast_math_request::automatic;
        #endif
}

#if FLTX_DETAIL_X86_FMA_RUNTIME_CHECK
bool bl::detail::fp::runtime_x86_fma_available_uncached() noexcept
{
    constexpr int bit_fma = 1 << 12;
    constexpr int bit_xsave = 1 << 26;
    constexpr int bit_osxsave = 1 << 27;
    constexpr int bit_avx = 1 << 28;
    constexpr int required = bit_fma | bit_xsave | bit_osxsave | bit_avx;

    #if defined(_MSC_VER)
    int regs[4]{};
    __cpuid(regs, 0);
    if (regs[0] < 1)
        return false;

    __cpuid(regs, 1);
    if ((regs[2] & required) != required)
        return false;

    return (_xgetbv(0) & 0x6) == 0x6;
    #else
    if (__get_cpuid_max(0, nullptr) < 1)
        return false;

    unsigned int eax{}, ebx{}, ecx{}, edx{};
    __cpuid(1, eax, ebx, ecx, edx);
    if ((static_cast<int>(ecx) & required) != required)
        return false;

    std::uint32_t xcr0_eax{};
    std::uint32_t xcr0_edx{};
    __asm__ volatile("xgetbv" : "=a"(xcr0_eax), "=d"(xcr0_edx) : "c"(0));
    const std::uint64_t xcr0 =
        (static_cast<std::uint64_t>(xcr0_edx) << 32u) | xcr0_eax;
    return (xcr0 & 0x6) == 0x6;
    #endif
}
#endif

const bl::detail::compiled_build_info& bl::detail::library_build_info() noexcept
{
    static constexpr compiled_build_info info{
        .fma_mode = configured_fma_policy,
        .fast_math_request = configured_fast_math_request,
        #if defined(FLTX_FAST_MATH)
        .fast_math_enabled = true,
        #else
        .fast_math_enabled = false,
        #endif
        .simd_policy_enabled = !FLTX_HEADER_SIMD_OFF,
        .x86_target = FLTX_X86_TARGET,
        .arm64_target = FLTX_ARM64_TARGET,
        .tu_has_x86_fma = FLTX_TU_HAS_X86_FMA,
        .has_x86_fma = FLTX_HAS_X86_FMA,
        .compiled_x86_fma_backend = FLTX_HAS_COMPILED_X86_FMA_BACKEND,
        .x86_fma_runtime_check = FLTX_DETAIL_X86_FMA_RUNTIME_CHECK,
        .msvc_guarded_x86_fma = FLTX_DETAIL_MSVC_GUARDED_X86_FMA,
        .scalar_x86_fma = FLTX_DETAIL_USE_SCALAR_X86_FMA,
        .baseline_arm64_fma = FLTX_DETAIL_USE_BASELINE_ARM64_FMA,
        .has_runtime_fma_path = FLTX_DETAIL_HAS_RUNTIME_FMA_PATH,
        .has_sse2 = FLTX_HAS_SSE2,
        .has_neon = FLTX_HAS_NEON,
        .has_wasm_simd = FLTX_HAS_WASM_SIMD,
        .f256_simd_enabled = FLTX_F256_ENABLE_SIMD,
        .f256_trig_simd_enabled = FLTX_F256_ENABLE_TRIG_SIMD,
        .simd_fma_two_prod = FLTX_SIMD_USE_FMA_TWO_PROD
    };
    return info;
}

bool bl::detail::library_runtime_hardware_fma_enabled() noexcept
{
    return fp::runtime_hardware_fma_enabled();
}
