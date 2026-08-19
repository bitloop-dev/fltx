/**
 * fltx/f256_approx_comparison.h - Approximate comparison for f256.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_F256_APPROX_COMPARISON_INCLUDED
#define FLTX_F256_APPROX_COMPARISON_INCLUDED

#include "fltx/f256_arithmetic.h"
#include "fltx/f256_classification.h"
#include "fltx/traits.h"
#include "fltx/detail/approx_comparison.h"

namespace bl
{
    inline constexpr f256_s f256_parity_tolerance{
        0x1p-188, 0.0, 0.0, 0.0
    };

    template<class Value, class Expected>
    requires (fltx_f256<Value> && fltx_f256<Expected>)
    [[nodiscard]] BL_FORCE_INLINE constexpr bool approx_eq(
        const Value& value,
        const Expected& expected,
        const f256_s& relative_tolerance = f256_parity_tolerance) noexcept
    {
        return detail::approx_eq_impl(
            static_cast<const f256_s&>(value),
            static_cast<const f256_s&>(expected),
            relative_tolerance);
    }
}

#endif
