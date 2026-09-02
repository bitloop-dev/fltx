/**
 * fltx/detail/build_info.h - Compiled-library build configuration reporting.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_DETAIL_BUILD_INFO_INCLUDED
#define FLTX_DETAIL_BUILD_INFO_INCLUDED

namespace bl::detail
{
    enum class configured_fma_mode
    {
        automatic,
        off,
        assume
    };

    enum class internal_fast_math_request
    {
        automatic,
        enabled,
        disabled
    };

    struct compiled_build_info
    {
        configured_fma_mode fma_mode;
        internal_fast_math_request fast_math_request;
        bool fast_math_enabled;
        bool simd_policy_enabled;
        bool x86_target;
        bool arm64_target;
        bool tu_has_x86_fma;
        bool has_x86_fma;
        bool compiled_x86_fma_backend;
        bool x86_fma_runtime_check;
        bool msvc_guarded_x86_fma;
        bool scalar_x86_fma;
        bool baseline_arm64_fma;
        bool has_runtime_fma_path;
        bool has_sse2;
        bool has_neon;
        bool has_wasm_simd;
        bool qd_simd_enabled;
        bool qd_trig_simd_enabled;
        bool simd_fma_two_prod;
    };

    [[nodiscard]] const compiled_build_info& library_build_info() noexcept;
    [[nodiscard]] bool library_runtime_hardware_fma_enabled() noexcept;
}

#endif
