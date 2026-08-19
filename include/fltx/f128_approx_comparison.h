/**
 * fltx/f128_approx_comparison.h - Approximate comparison for f128.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_F128_APPROX_COMPARISON_INCLUDED
#define FLTX_F128_APPROX_COMPARISON_INCLUDED

#include "fltx/f128_arithmetic.h"
#include "fltx/f128_classification.h"
#include "fltx/traits.h"
#include "fltx/detail/approx_comparison.h"

namespace bl
{
    inline constexpr f128_s f128_parity_tolerance{ 0x1p-80, 0.0 };

    template<class Value, class Expected>
    requires (fltx_f128<Value> && fltx_f128<Expected>)
    [[nodiscard]] BL_FORCE_INLINE constexpr bool approx_eq(
        const Value& value,
        const Expected& expected,
        const f128_s& relative_tolerance = f128_parity_tolerance) noexcept
    {
        return detail::approx_eq_impl(
            static_cast<const f128_s&>(value),
            static_cast<const f128_s&>(expected),
            relative_tolerance);
    }
}

#endif
