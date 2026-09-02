/**
 * fltx/f64_approx_comparison.h - Approximate comparison for f64 values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_F64_APPROX_COMPARISON_INCLUDED
#define FLTX_F64_APPROX_COMPARISON_INCLUDED

#include "fltx/f64_classification.h"
#include "fltx/traits.h"
#include "fltx/detail/approx_comparison.h"

namespace bl
{
    // Preserve the native validation floor of 48 significant bits while
    // allowing small runtime/constant-evaluation path differences.
    inline constexpr f64 f64_parity_tolerance = 0x1p-48;

    template<class Value, class Expected>
    requires (fltx_f64<Value> && fltx_f64<Expected>)
    [[nodiscard]] BL_FORCE_INLINE constexpr bool approx_eq(
        const Value& value,
        const Expected& expected,
        f64 relative_tolerance = f64_parity_tolerance) noexcept
    {
        return detail::approx_eq_impl(value, expected, relative_tolerance);
    }
}

#endif
