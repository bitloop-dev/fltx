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
    template<class Value, class Expected>
    requires (fltx_f128<Value> && fltx_f128<Expected>)
    [[nodiscard]] BL_FORCE_INLINE constexpr bool almost_equal(
        const Value& value,
        const Expected& expected,
        const f128_s& relative_tolerance = f128_s{ 0x1p-74, 0.0 },
        const f128_s& absolute_tolerance = f128_s{ 0.0, 0.0 }) noexcept
    {
        return detail::almost_equal_impl(
            static_cast<const f128_s&>(value),
            static_cast<const f128_s&>(expected),
            relative_tolerance,
            absolute_tolerance);
    }
}

#endif
