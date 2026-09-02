/**
 * fltx/f32_approx_comparison.h - Approximate comparison for f32 values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_F32_APPROX_COMPARISON_INCLUDED
#define FLTX_F32_APPROX_COMPARISON_INCLUDED

#include "fltx/f32_classification.h"
#include "fltx/traits.h"
#include "fltx/detail/approx_comparison.h"

namespace bl
{
    // Preserve the native validation floor of 20 significant bits while
    // allowing small runtime/constant-evaluation path differences.
    inline constexpr f32 f32_parity_tolerance = 0x1p-20f;

    template<class Value, class Expected>
    requires (fltx_f32<Value> && fltx_f32<Expected>)
    [[nodiscard]] BL_FORCE_INLINE constexpr bool approx_eq(
        const Value& value,
        const Expected& expected,
        f32 relative_tolerance = f32_parity_tolerance) noexcept
    {
        return detail::approx_eq_impl(value, expected, relative_tolerance);
    }
}

#endif
